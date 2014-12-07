/*
 * Known bugs:
 * Functions cannot pass objects by value, only by pointer
 * If user returns an array with new int[i] the wrong deleter will be called.
 * If user return a non-pointer of size pointer, it will convert that to a pointer and dereferenced[seg fault]
 * If user uses the wrong return type, it will cast to that return type. 
 * If the user leaves out the return type and the function returns, a memory leak will occur
 * For safety reasons, consider storing typeid of every value stored
 * For performance reasons, consider replacing void * with unique *
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

// out_of_range exceptions are thrown by unordered_map
#include <unordered_map>

//For typeid
#include <typeinfo>

//For bad_function_call
#include <functional>

// Function pointer cast produces a function that takes in an arbitrary # of args and returns a void pointer.
typedef void * (*pcast) (...);
#define ____OBJECT_TYPE -1111111111111111111LL

/*
 * Dynamic object which is capable of adding static function pointers and values to itself.
 */
class Object {
    // type is stored as a 64 bit value at the head of an object
    const long long my_type = ____OBJECT_TYPE;

    // contents table stores persistent variables and other objects
    std::unordered_map<std::string, void * > my_contents;
public:
    // re-assignable pointer to parent of this object. Function in this class will use this pointer to go up the inheritance hierarchy.
    Object * my_parent;

    // re-assignable function pointer - set with Object::setFunc and called with Object::call<Return_Type>
    pcast execute_me;

    // Default constructor

    Object() {
        my_parent = nullptr;
        my_contents = {};
        execute_me = nullptr;
    }

    //Frees all the memory used to store items in hash table.

    ~Object() {
        for (auto it = my_contents.begin(); it != my_contents.end(); ++it) {
            delete it->second;
        }
    }
    Object& operator =(const Object &other) {
        this->my_contents = other.my_contents;
        this->my_parent = other.my_parent;
        this->execute_me = other.execute_me;
        return *this;
    }

    /*
     * Sets function pointer execute_me to the address of a static function.
     * @param function_pointer - a generic function pointer
     */
    template <class Type> void setFunc(Type function_pointer) {

        //Prevents the user from setting function pointer to an object
        if (function_pointer == nullptr || (sizeof (function_pointer) != sizeof (this->execute_me) )) {
            std::cerr << "Function pointer is null." << std::endl;
            throw std::bad_function_call();
            return;
        } else {
            this->execute_me = (pcast) function_pointer;
            return;
        }
    }

    // Add a copy of a single object property to the properties hash table.

    /*
     * Add a single object property to the properties hash table with key string::name and generic value. Will leak memory if you pass in new int[] allocate array.
     * @param name - name that will be used to retrieve value
     * @param value - a generic value to be added
     */
    template <class Type> void set(const std::string name, const Type &value) {
        Type * toSet = new Type;
        *toSet = value;
        my_contents[name] = (void *) (toSet);
        return;
    }

    /*
     * Checks to see if this object has a variable with name value equal to std::string name
     * Throws out_of_range exception when name cannot be found
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

    /*
     * Retrieves an element from this object with non-void return type
     * Throws out_of_range exception when name cannot be found
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
                std::cerr << "Member not found." << std::endl;
                throw std::out_of_range("Member not found.");
                Return_Type r;
                return r;
            }
        }
    }

    /*
     * Directly calls the generic function pointer execute_me if the return type is void.
     * Throws bad function call if pointer is null. Will leaks memory if used with a non-void  function.
     * @param Parameters - generic list of function parameters
     */
    template <class ...A> void call(A... Parameters) {
        if (this->execute_me != nullptr) {
            (this->execute_me(Parameters...)); // It is NOT safe to delete a null pointer - memory will leak if the user messes up the return type
            return;
        } else if (this->my_parent != nullptr) {
            this->my_parent->call(Parameters...);
            return;
        } else {
            std::cerr << "Function pointer passed to call is nullptr." << std::endl;
            throw std::bad_function_call();
            return;
        }
    }

    /*
     * Directly calls the generic function pointer execute_me contained within this object if the return type is non-void
     * Throws bad function call if pointer is null
     * @param Parameters - generic list of generic list of comma delimited function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template <class Return_Type, class ...A> Return_Type call(A... Parameters) {
        if (this->execute_me != nullptr) {
            ///*
            Return_Type * rptr = (Return_Type *) (this->execute_me(Parameters...)); //get something allocated with new
            Return_Type ret = *rptr; //copy it
            delete rptr;
            return ret; //return the copy. If this thing returned a unique pointer, copying would be unnecessary.
        } else if (this->my_parent != nullptr) {
            return this->my_parent->call<Return_Type>(Parameters...);
        } else {
            std::cerr << "Function pointer passed to call is nullptr." << std::endl;
            throw std::bad_function_call();
            Return_Type r;
            return r;
        }
    }

    /*
     * Executes a function with no return type by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     */
    template<class ...A> void Do(const std::string function_name, A... Parameters) {
        try {
            Object * isObject = (Object *) (my_contents.at((std::string) function_name));
            if (!(isObject->my_type == ____OBJECT_TYPE)) {
                std::cerr << "Element referenced by std::string function_name cannot use an object function call." << std::endl;
                throw std::bad_function_call();
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
                std::cerr << "Function could not be found." << std::endl;
                throw std::out_of_range("Function could not be found.");
                return;
            }
        }
    }

    /*
     * Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class Return_Type, class ...A> Return_Type Do(const std::string function_name, A... Parameters) {

        try {
            Object * isObject = (Object *) (my_contents.at((std::string) function_name));

            if (!(isObject->my_type == ____OBJECT_TYPE)) {
                std::cerr << "Element referenced by std::string function_name cannot use an object function call." << std::endl;
                throw std::bad_function_call();
                Return_Type r;
                return r;
            }

            //Calls the corresponding function in the hash table of function objects.

            return isObject->call<Return_Type>(Parameters...);
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            std::cout << "Function not found" << std::endl;
            if (this->my_parent != nullptr)
                return this->my_parent->Do<Return_Type>(function_name, Parameters...);
                //Give up.
            else {
                std::cerr << "Function could not be found." << std::endl;
                throw std::out_of_range("Function could not be found.");
                Return_Type r;
                return r;
            }
        }
    }

};

//#define a auto
#define s std::string

int main() {

    //Prototypal_C is a header implementing a class that allows users to write dynamic, type-safe, prototypal inheritance based c++ code. The Object class contained within this header can be instantiated to create generic containers capable of augmenting themselves with members and functions of various types. Members are accessed indirectly by passing a string to a "get" function. 

//Example: 
  Object object;
  int x = 5;
  object.set("x",x);              // add a new member x to object and set its name to "x".
  int i = object.get<int>("x");   // get member of type int whose name is "x".
  std::cout << i << std::endl;    // print 5.

//===================================================================================================================

//One member function can be called directly using the "call" method, which avoids the overhead of object storage and retrieval. This feature allows members of type Object to have their own persistent variables.

//Example:
  struct ss{static void print() {std::cout << "hello world" << std::endl;} };
  object.setFunc(ss::print);  // sets object's function pointer to the print function.
  object.call();                  // directly calls the print function. 


//===================================================================================================================


//Note that only static global functions, non-static global functions, and non-static class member functions can be passed using setFunc() and call(). Member functions setFunc() and call() can also be used to pass parameters of primitive types or pointers to class types. 

  
  
//Example:
  struct vv {static void func(Object * o, int x) {} };
  object.setFunc(vv::func );    // sets object's function pointer to the func function.
  Object * ob = &object;
  object.call(ob, x);        // directly calls the func function.
//===================================================================================================================

 
    
//Also note that for functions returning non-void, the return type must be a pointer whose contents will be allocated on heap.

//Example:
  struct ww {static int * add(int x, int y) {return new int(x+y);} };
  object.setFunc(ww::add);      // sets object's function pointer to the add function.
  x = object.call<int>(5, 6);     // returns 11. The dereferenced return type is specified in angle brackets.
  std::cout << x << std::endl;
    
//===================================================================================================================


//Members of type Object can designate a parent and pass searches for variables and function calls to their parent.

//Example:
  Object child;
  child.my_parent = &object;
  object.my_parent = nullptr;
  std::cout << object.get<int>("x") << std::endl;           // gets the member of type int whose name is "x" from object. prints 5.
  

//==================================================================================================================

  
//Direct function calls can also be designated object parent if they are equal to the default value, nullptr, in the child.

//Example:
  std::cout << child.call<int>(5, 6) << std::endl;         // returns 11. The dereferenced return type is specified in angle brackets.


//===================================================================================================================


//Indirect function calls can also be performed. Objects which can directly call a function can be placed inside of other objects using the "object.set(std::string name, Type T)" method. An outer object can call an inner object by with the method "object.Do<Return_Type T>(std::string name, Parameter_Pack P)". 

//Example:
  object.set("child", child);     // add a new member child to object and set its name to "child". 
  std::cout << object.Do<int>("child", 5, 6) << std::endl;  // tells member whose name is "child" to perform the call function. 
  
 
//===================================================================================================================

  
  //Note that "Object child" is inside of "Object object" and that "Object child" has access to "Object object". This pattern can also be applied to subclasses of Object. 
  
//For Example:

  class Computer : public Object {};
  class Printer : public Object {};
  Computer comp;
  Printer p;
  p.setFunc(ss::print);
  comp.set("print",p);
  comp.Do("print");               // Computer calls Printer's print function.
    
    return 0;
}
