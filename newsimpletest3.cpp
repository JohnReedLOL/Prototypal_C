/* 
 * File:   newsimpletest3.cpp
 * Author: johnmichaelreed2
 *
 * Created on Dec 5, 2014, 9:10:20 PM
 */

#include <stdlib.h>



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
     * @brief Directly calls the generic function pointer execute_me contained within this object if the return type is non-void. Throws bad function call if pointer is null. If you use this with a void function, the code will not compile.
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

bool is_object(Object* value);

//global object that all the tests will use
Object obj;

void testIs_object() {
    //Object* value;
    //bool result = is_object(value);
    //if (true /*check result*/) {
    //    std::cout << "%TEST_FAILED% time=0 testname=testIs_object (newsimpletest3) message=error message sample" << std::endl;
    //}
}

void testDo() {
    char b = 2;
    obj.set("lala", b);

    try {
        obj.Do("lala", 1, 2, 3, 4); //This should throw an exception because b is not a function object
        std::cout << "%TEST_FAILED% time=0 testname=testDo (newsimpletest3) message=error message sample" << std::endl;
    } catch (std::bad_function_call ee) {
        std::cout << "good1" << std::endl; //expected output
    } catch (std::out_of_range f) {
        std::cout << "good2" << std::endl;
    }
}

void testDo2() {
    char b = 2;
    obj.set("lala", b);

    try {
        obj.Do<int>("lala", 1, 2, 3, 4); //This should throw an exception because b is not a function object
        std::cout << "%TEST_FAILED% time=0 testname=testDo (newsimpletest3) message=error message sample" << std::endl;
    } catch (std::bad_function_call ee) {
        std::cout << "good1" << std::endl; //expected output
    } catch (std::out_of_range f) {
        std::cout << "good2" << std::endl;
    }
}

void testObject() {
    Object object1;
    if (! (object1.my_type == ____OBJECT_TYPE)) {
        std::cout << "%TEST_FAILED% time=0 testname=testObject (newsimpletest3) message=error message sample" << std::endl;
    }
}

void printv() {
    std::cout << "print" << std::endl;
    return;
}

void testCall() {
    try {
        obj.setFunc(::printv);
        obj.call(); //try this with a void returning function or with parameter void and see what happens.
    } catch (std::exception x) {
        std::cout << "%TEST_FAILED% time=0 testname=testCall2 (newsimpletest3) message=error message sample" << std::endl;
    }
}

int print() {
    std::cout << "print" << std::endl;
    return 5;
}

void testCall2() {

    obj.setFunc(::print);
    int ret = obj.call<int>();

    if (!(ret == 5)) {
        std::cout << "%TEST_FAILED% time=0 testname=testCall (newsimpletest3) message=error message sample" << std::endl;
    }
}
/*
void testGet() {
    const std::string& name;
    Object object;
    Return_Type result = object.get(name);
    if (true ) {
        std::cout << "%TEST_FAILED% time=0 testname=testGet (newsimpletest3) message=error message sample" << std::endl;
    }
}

void testHas() {
    const std::string& name;
    Object object;
    bool result = object.has(name);
    if (true ) {
        std::cout << "%TEST_FAILED% time=0 testname=testHas (newsimpletest3) message=error message sample" << std::endl;
    }
}

void testSet() {
    const std::string name;
    const Type& value;
    Object object;
    object.set(name, value);
    if (true ) {
        std::cout << "%TEST_FAILED% time=0 testname=testSet (newsimpletest3) message=error message sample" << std::endl;
    }
}

void testSetFunc() {
    Type function_pointer;
    Object object;
    object.setFunc(function_pointer);
    if (true ) {
        std::cout << "%TEST_FAILED% time=0 testname=testSetFunc (newsimpletest3) message=error message sample" << std::endl;
    }
}
*/

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% newsimpletest3" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% testIs_object (newsimpletest3)" << std::endl;
    testIs_object();
    std::cout << "%TEST_FINISHED% time=0 testIs_object (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testDo (newsimpletest3)" << std::endl;
    testDo();
    std::cout << "%TEST_FINISHED% time=0 testDo (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testDo2 (newsimpletest3)" << std::endl;
    testDo2();
    std::cout << "%TEST_FINISHED% time=0 testDo2 (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testObject (newsimpletest3)" << std::endl;
    testObject();
    std::cout << "%TEST_FINISHED% time=0 testObject (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testCall (newsimpletest3)" << std::endl;
    testCall();
    std::cout << "%TEST_FINISHED% time=0 testCall (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testCall2 (newsimpletest3)" << std::endl;
    testCall2();
    std::cout << "%TEST_FINISHED% time=0 testCall2 (newsimpletest3)" << std::endl;

    /*
    
    std::cout << "%TEST_STARTED% testGet (newsimpletest3)" << std::endl;
    testGet();
    std::cout << "%TEST_FINISHED% time=0 testGet (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testHas (newsimpletest3)" << std::endl;
    testHas();
    std::cout << "%TEST_FINISHED% time=0 testHas (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testSet (newsimpletest3)" << std::endl;
    testSet();
    std::cout << "%TEST_FINISHED% time=0 testSet (newsimpletest3)" << std::endl;

    std::cout << "%TEST_STARTED% testSetFunc (newsimpletest3)" << std::endl;
    testSetFunc();
    std::cout << "%TEST_FINISHED% time=0 testSetFunc (newsimpletest3)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;
     
    */

    return (EXIT_SUCCESS);
}

