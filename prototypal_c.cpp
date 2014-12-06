/*
 * Known bugs:
 * Functions cannot pass objects by value
 * If user returns an array with new int[i] the wrong deleter will be called.
 * If set passes a non-pointer of size pointer [or if function returns a non-pointer of size pointer], program will convert non-pointer to pointer and dereference [seg fault]
 * If user extracts a type from a stored void function, an error will result
 * If user uses the wrong return type, it will cast to that return type. 
 * 
 */

/*
Copyright [2014] [John-Michael Reed]
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

/* 
 * File:   main.cpp
 * Author: johnmichaelreed2
 *
 * Created on November 29, 2014, 3:35 PM
 */

#include <iostream>
#include <functional>

#include <unordered_map>

// out_of_range exceptions are thrown by unordered_map
#include <exception>

using namespace std;
typedef void * (*pcast) (...);
#define ____OBJECT_TYPE -1111111111111111111LL
/**
 * @brief Dynamic object which is capable of adding static function pointers and values to itself.
 */
struct Object {
    // type is stored as a 64 bit value at the head of an object
    const long long my_type = ____OBJECT_TYPE;

    // contents table stores persistent variables and other objects
    std::unordered_map<std::string, void * > my_contents;

    // re-assignable pointer to parent of this object. Function in this class will use this pointer to go up the inheritance hierarchy.
    Object * my_parent;

    // re-assignable function pointer - set with Object::setFunc and called with Object::call<Return_Type>
    pcast execute_me;

    // Default constructor
    Object() {
        my_parent = nullptr;
        my_contents = {};
    }

    //Frees all the memory used to store items in hash table.
    ~Object() {
        for (auto it = my_contents.begin(); it != my_contents.end(); ++it) {
            delete it->second;
        }
    }

    /**
     * @brief Sets function pointer execute_me to the address of a static function.
     * @param function_pointer - a generic function pointer
     */
    template <class Type> void setFunc(Type function_pointer) {

        //Prevents the user from setting function pointer to an object
        if (function_pointer == nullptr || (sizeof (function_pointer) != sizeof (this->execute_me))) {
            throw std::bad_function_call();
            return;
        } else {
            this->execute_me = (pcast) function_pointer;
            return;
        }
    }

    /**
     * @brief Add a single object property to the properties hash table with key string::name and generic value. Will leak memory if you pass in new int[] allocate array.
     * @param name - name that will be used to retrieve value
     * @param value - a generic value to be added
     */
    template <class Type> void set(const std::string name, const Type &value) {
        Type * toSet = new Type;
        *toSet = value;
        my_contents[name] = (void *) (toSet);
    }

    /**
     * @brief Checks to see if this object has a variable with name value equal to std::string name. Throws out_of_range exception when name cannot be found.
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an object somewhere in its parent tree.
     */
    bool has(const std::string &name) {

        //If the element exists, get it.
        try {
            ((this->my_contents.at(name)));
            return true;
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->has(name);
            } else {
                return false;
            }
        }
    }

    /**
     * @brief Retrieves an element from this object with non-void return type. Throws out_of_range exception when name not found.
     * @param name - string name of the variable that we are searching for
     * @return Return_Type - return type which must be specified in angle brackets
     */
    template <class Return_Type> Return_Type get(const std::string &name) {

        //If the element exists, get it.
        try {
            return * ((Return_Type *) (this->my_contents.at(name)));
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get<Return_Type>(name);
            } else {
                throw std::out_of_range("Member not found.");
                Return_Type ERROR;
                return ERROR;
            }
        }
    }

    /**
     * @brief Directly calls the generic function pointer execute_me if the return type is void. Throws bad function call if pointer is null. Will leaks memory if used with a non-void  function.
     * @param Parameters - generic list of function parameters
     */
    template <class ...A> void call(A... Parameters) {
        if (this->execute_me != nullptr) {
            delete (this->execute_me(Parameters...)); // It is safe to delete a null pointer - effectively amounts to a no-op.
            return;
        } else if (this->my_parent != nullptr) {
            this->my_parent->call(Parameters...);
            return;
        } else {
            throw std::bad_function_call();
        }
    }

    /**
     * @brief Directly calls the generic function pointer execute_me contained within this object if the return type is non-void. hrows bad function call if pointer is null. If you use this with a function that does not return with new, an error must occur.
     * @param Parameters - generic list of generic list of comma delimited function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template <class Return_Type, class ...A> Return_Type call(A... Parameters) {
        if (this->execute_me != nullptr) {
            Return_Type * rptr = (Return_Type *) (this->execute_me(Parameters...)); //get something allocated with new
            Return_Type ret = *rptr; //copy it
            delete (void *) rptr;
            return ret; //return the copy. If this thing returned a unique pointer, copying would be unnecessary to avoid undefined behavior
        } else if (this->my_parent != nullptr) {
            return this->my_parent->call<Return_Type>(Parameters...);
        } else {
            throw std::bad_function_call();
        }
    }

    /**
     * @brief Executes a function with no return type by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     */
    template<class ...A> void Do(const std::string function_name, A... Parameters) {

        try {
            Object * isObject = (Object *) (my_contents.at((std::string) function_name));

            if (!(isObject->my_type == ____OBJECT_TYPE)) {
                throw std::bad_function_call(); // std::bad_function_call("Element referenced by std::string function_name cannot use an object function call.");
                return;
            }

            //Calls the corresponding function in the hash table of function objects.

            isObject->call(Parameters...);
            return;
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {

            //Check my_parent.
            if (this->my_parent != nullptr) {
                this->my_parent->Do(function_name, Parameters...);
                return;
            }//Give up.
            else {
                throw std::out_of_range("Function cound not be found.");
                return;
            }
        }
    }

    /*
     * @brief Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class Return_Type, class ...A> Return_Type Do(const std::string function_name, A... Parameters) {

        std::cout << "Endering Do<>" << std::endl;
        try {
            Object * isObject = (Object *) (my_contents.at((std::string) function_name));

            std::cout << "Object accessed" << std::endl;
            std::cout << isObject->my_type << std::endl;

            if (!(isObject->my_type == ____OBJECT_TYPE)) {
                std::cout << "Seg fault..............." << std::endl;
                throw std::bad_function_call();
                Return_Type r;
                return r;
            }

            std::cout << "Type checked" << std::endl;
            //Calls the corresponding function in the hash table of function objects.

            return isObject->call<Return_Type>(Parameters...);
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            std::cout << "Value not found" << std::endl;
            if (this->my_parent != nullptr)
                return this->my_parent->Do<Return_Type>(function_name, Parameters...);
                //Give up.
            else {
                throw std::out_of_range("Function cound not be found.");
                Return_Type r;
                return r;
            }
        }
    }

};
