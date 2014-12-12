#ifndef PROTOTYPAL_CPP
#define PROTOTYPAL_CPP

/*
 * Known bugs:
 * Functions cannot pass objects by value, only by pointer. Will not compile if object is passed by value.
 * If user return a non-pointer of size pointer, it will convert that to a pointer and dereferenced[seg fault]. 
 * If user uses the wrong return type, it will cast to that return type. Use of return typeid storage can prevent this.
 * Keep track of size of each object?.
 * 
 * Replace dynamic type checking with trailing return types!!!!!!!!!!!!!!!
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

#include <typeindex>      // std::type_index wrapper for type_info
#include <iostream>
#include <unordered_map>
#include <functional> //For bad_function_call
#include <memory> // shared_ptr, typeid

#define ____OBJECT_TYPE -9223372036854775807LL
#define ____ACTION_TYPE -9223372036854775808LL

// Function pointer cast produces a function that takes in an arbitrary # of args and returns a void pointer.
typedef void * (*pcast) (...);

/*
 * Dynamic object which is capable of adding static function pointers and values to itself.
 */
class Object_Prototype {
    //protected:
public:
    // type is stored as a 64 bit value at the head of an object
    const int64_t my_type;

    struct Shared_Pointer_And_Type {
        std::shared_ptr<void> p;
        std::type_index t;

        Shared_Pointer_And_Type() : p(nullptr), t(typeid (void)) {
        }

        Shared_Pointer_And_Type(const std::shared_ptr<void>pp, const std::type_index tt) : p(pp), t(tt) {
        }

        Shared_Pointer_And_Type(const Shared_Pointer_And_Type &other) : p(other.p), t(other.t) {
        }

        Shared_Pointer_And_Type& operator =(const Shared_Pointer_And_Type &other) {
            this->p = other.p;
            this->t = other.t;
            return *this;
        }
    };
    // contents table stores persistent variables and other objects
    std::unordered_map<std::string, Object_Prototype::Shared_Pointer_And_Type> my_contents; //When a shared_ptr is copied (or default constructed) from another the deleter is passed around, so that when you construct a shared_ptr<T> from a shared_ptr<U> the information on what destructor to call is also passed around in the deleter
public:
    // re-assignable function pointer - set with Object_Prototype::setFunc and called with Object_Prototype::call<Return_Type>
    pcast execute_me;
    // re-assignable pointer to parent of this object. Function in this class will use this pointer to go up the inheritance hierarchy.
    Object_Prototype * my_parent;
    // Default constructor

    Object_Prototype() : my_type(____OBJECT_TYPE), my_contents(), execute_me(nullptr), my_parent(nullptr) {
    }

    Object_Prototype(const Object_Prototype &o) : my_type(o.my_type), my_contents(o.my_contents), execute_me(o.execute_me), my_parent(o.my_parent) {
    }
protected:

    Object_Prototype(bool b) : my_type(____ACTION_TYPE), my_contents(), execute_me(nullptr), my_parent(nullptr) {
    }
public:

    virtual ~Object_Prototype() {
    }

    Object_Prototype& operator =(const Object_Prototype &other) {
        this->my_contents = other.my_contents;
        this->my_parent = other.my_parent;
        this->execute_me = other.execute_me;
        return *this;
    }

    void pass_contents(const Object_Prototype &other) {
        this->my_contents = other.my_contents;
    }

    /*
     * Sets function pointer execute_me to the address of a static function. 
     * @param function_pointer - a generic function pointer
     */
    template <class Type> void setFunc(Type function_pointer) {
        //Prevents the user from setting function pointer to an object
        if (function_pointer == nullptr || (sizeof (function_pointer) != sizeof (this->execute_me))) {
            std::cerr << "Function pointer is null or function cannot safely be assigned." << std::endl;
            throw std::bad_function_call();
            return;
        } else {
            this->execute_me = (pcast) function_pointer;
            return;
        }
    }

    /*
     * Add a single object property to the properties hash table with key string::name and generic value. Will leak memory if you pass in new int[] allocate array.
     * @param name - name that will be used to retrieve value
     * @param value - a generic value to be added
     */
    template <class Type> void set(const std::string &name, const Type &value) {
        std::shared_ptr<Type> shared_pointer = std::make_shared<Type>(value);
        Shared_Pointer_And_Type temp(std::static_pointer_cast<void>(shared_pointer), std::type_index(typeid (value)));
        this->my_contents[name] = temp;
        return;
    }

    //alias for set
    template <typename... Args>
    auto add(Args&&... args) -> decltype(set(std::forward<Args>(args)...)) {
        return set(std::forward<Args>(args)...);
    }

    /*
     * Checks to see if this object has a variable with name value equal to std::string name
     * Throws out_of_range exception when name cannot be found
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an object somewhere in its parent tree.
     */
    bool has(const std::string &name) {
        try { //If the element exists, get it.
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
     * Checks to see if this object has a variable with name value equal to std::string name and type equal to Element_Type
     * Throws out_of_range exception when name cannot be found
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an object somewhere in its parent tree.
     */
    template <class Element_Type> bool has(const std::string &name) {
        try { //If the element exists, get it.
            Shared_Pointer_And_Type spt = this->my_contents.at(name);
            if (spt.t == std::type_index(typeid (Element_Type))) {
                return true;
            } else {
                return false;
            }
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {
            if (this->my_parent != nullptr) {
                return this->my_parent->has<Element_Type>(name);
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
            Shared_Pointer_And_Type spt = this->my_contents.at(name);
            if (spt.t == std::type_index(typeid (Return_Type))) {
                Return_Type r = *(std::static_pointer_cast<Return_Type>(spt.p));
                return r;
            } else {
                std::cerr << "Type specified in get<Return_Type> function does not match a retrievable member's type." << std::endl;
                throw std::bad_cast();
                Return_Type r;
                return r;
            }
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {
            if (this->my_parent != nullptr) {
                return this->my_parent->get<Return_Type>(name);
            } else {
                std::cerr << "Member not found." << std::endl;
                throw oor;
                //Return_Type r;
                //return r;
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
            Return_Type * rptr = (Return_Type *) (this->execute_me(Parameters...)); //get something allocated with new
            Return_Type ret = *rptr; //copy it
            delete rptr;
            return ret; //return the copy. If this thing returned a unique pointer, copying would be unnecessary.
        } else if (this->my_parent != nullptr) {
            return this->my_parent->call<Return_Type>(Parameters...);
        } else {
            std::cerr << "Function pointer passed to call is nullptr." << std::endl;
            throw std::bad_function_call();
            //Return_Type r;
            //return r;
        }
    }
    
    

    /*
     * Executes a function with no return type by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     */
    template<class ...A> void exec(const std::string &function_name, A... Parameters) {
        try {
            Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at((std::string) function_name);
            std::shared_ptr<Object_Prototype> isObject_Prototype = std::static_pointer_cast<Object_Prototype>(spt.p);
            if ((isObject_Prototype->my_type > ____OBJECT_TYPE)) {
                std::cerr << "Element referenced by std::string function_name cannot use an object function call." << std::endl;
                throw std::bad_function_call();
                return;
            }
            //Call the corresponding function in the hash table of function objects.
            isObject_Prototype->call(Parameters...);
            return;
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {

            //Check my_parent.
            if (this->my_parent != nullptr) {
                this->my_parent->exec(function_name, Parameters...);
                return;
            }//Give up.
            else {
                std::cerr << "Function could not be found." << std::endl;
                throw oor;
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
    template<class Return_Type, class ...A> Return_Type exec(const std::string &function_name, A... Parameters) {
        try {
            Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at((std::string) function_name);
            std::shared_ptr<Object_Prototype> isObject_Prototype = std::static_pointer_cast<Object_Prototype>(spt.p);
            if ((isObject_Prototype->my_type > ____OBJECT_TYPE)) {
                std::cerr << "Element referenced by std::string function_name cannot use an object function call." << std::endl;
                throw std::bad_function_call();
                //Return_Type r;
                //return r;
            }
            //Calls the corresponding function in the hash table of function objects.
            return isObject_Prototype->call<Return_Type>(Parameters...);
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            std::cout << "Function not found" << std::endl;
            if (this->my_parent != nullptr)
                return this->my_parent->exec<Return_Type>(function_name, Parameters...);
                //Give up.
            else {
                std::cerr << "Function could not be found." << std::endl;
                throw oor;
                //Return_Type r;
                //return r;
            }
        }
    }

    template<class Standard_Function, class Return_Type = void, class ...A> Return_Type lexec(const std::string &function_name, A... Parameters) {
        try {
            Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at(function_name);
            Standard_Function isLambda = *(std::static_pointer_cast<Standard_Function>(spt.p));
            return isLambda(Parameters...);
        } catch (std::out_of_range e) {
            if (this->my_parent != nullptr) {
                return lexec<Standard_Function, Return_Type>(function_name, Parameters...);
            } else {
                throw e;
            }

        }
    }

};

/*
 * Dynamic object which is capable of adding static function pointers and values to itself.
 */
template <class Lambda> class Function : public Object_Prototype {
public:
    const Lambda Do;

    // Default constructor
    Function() : Object_Prototype(false), Do([] {
    }) {
    }

    explicit Function(Lambda l) : Object_Prototype(false), Do(l) {
    }

    Function(const Function<Lambda> &o) : Object_Prototype(false), Do(o.Do) {
    }

    /*
     * Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class New_Lambda, class Return_Type, class ...A> Return_Type fexec(const std::string &function_name, A... Parameters) {
        Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at(function_name);
        std::shared_ptr<Function < New_Lambda>> isFunction = std::static_pointer_cast<Function < New_Lambda >> (spt.p);
        if ((isFunction->my_type != ____ACTION_TYPE)) {
            std::cerr << "Element referenced by std::string function_name cannot use a lambda object function call." << std::endl;
            throw std::bad_function_call();
            //Return_Type r;
            //return r;
        }
        //Calls the corresponding function in the hash table of function objects.
        return isFunction->Do(Parameters...);
    }

    /*
     * Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class New_Lambda, class ...A> void fexec(const std::string &function_name, A... Parameters) {
        Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at(function_name);
        std::shared_ptr<Function < New_Lambda>> isFunction = std::static_pointer_cast<Function < New_Lambda >> (spt.p);
        if ((isFunction->my_type != ____ACTION_TYPE)) {
            std::cerr << "Element referenced by std::string function_name cannot use a lambda object function call." << std::endl;
            throw std::bad_function_call();
            //Return_Type r;
            //return r;
        }
        //Calls the corresponding function in the hash table of function objects.
        return isFunction->Do(Parameters...);
    }

    virtual ~Function() {
    }
};

/*
 * Dynamic object which is capable of adding static function pointers and values to itself.
 */
class Object : public Object_Prototype {
public:

    // Default constructor

    Object() : Object_Prototype() {
    }

    Object(const Object_Prototype &o) : Object_Prototype(o) {
    }

    /*
     * Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class New_Lambda, class Return_Type, class ...A> Return_Type fexec(const std::string &function_name, A... Parameters) {
        Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at(function_name);
        std::shared_ptr<Function < New_Lambda>> isFunction = std::static_pointer_cast<Function < New_Lambda >> (spt.p);
        if ((isFunction->my_type != ____ACTION_TYPE)) {
            std::cerr << "Element referenced by std::string function_name cannot use a lambda object function call." << std::endl;
            throw std::bad_function_call();
            //Return_Type r;
            //return r;
        }
        //Calls the corresponding function in the hash table of function objects.
        return isFunction->Do(Parameters...);
    }

    /*
     * Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class New_Lambda, class ...A> void fexec(const std::string &function_name, A... Parameters) {
        Object_Prototype::Shared_Pointer_And_Type spt = my_contents.at(function_name);
        std::shared_ptr<Function < New_Lambda>> isFunction = std::static_pointer_cast<Function < New_Lambda >> (spt.p);
        if ((isFunction->my_type != ____ACTION_TYPE)) {
            std::cerr << "Element referenced by std::string function_name cannot use a lambda object function call." << std::endl;
            throw std::bad_function_call();
            //Return_Type r;
            //return r;
        }
        //Calls the corresponding function in the hash table of function objects.
        return isFunction->Do(Parameters...);
    }

    virtual ~Object() {
    }
};

#endif

//#define Object Object<>

//#define a auto
#define s std::string

int main() {
    //Prototypal_C is a header implementing a class that allows users to write dynamic, type-safe, prototypal inheritance based c++ code. The Object class contained within this header can be instantiated to create generic containers capable of augmenting themselves with members and functions of various types. Members are accessed indirectly by passing a string to a "get" function. 
    //Example: 
    Object object;
    int x = 5;
    object.set("x", x); // add a new member x to object and set its name to "x".
    int i = object.get<int>("x"); // get member of type int whose name is "x".
    std::cout << i << std::endl; // print 5.
    std::cout << "object has \"x\": " << object.has("x") << std::endl;
    std::cout << "object has int x: " << object.has<int>("x") << std::endl;
    std::cout << "object has float x: " << object.has<float>("x") << std::endl;
    //===================================================================================================================
    //One member function can be called directly using the "call" method, which avoids the overhead of object storage and retrieval. This feature allows members of type Object to have their own persistent variables.
    //Example:

    struct ss {

        static void print() {
            std::cout << "hello world" << std::endl;
        }
    };
    object.setFunc(ss::print); // sets object's function pointer to the print function.
    object.call(); // directly calls the print function. 
    //===================================================================================================================
    //Note that only static global functions, non-static global functions, and non-static class member functions can be passed using setFunc() and call(). Member functions setFunc() and call() can also be used to pass parameters of primitive types or pointers to class types. 
    //Example:

    struct vv {

        static void func(Object * o, int x) {
            ++x;
            o->set("new_x", x);
        }
    };
    object.setFunc(vv::func); // sets object's function pointer to the func function.
    Object * ob = &object;
    object.call(ob, x); // directly calls the func function.

    int iii = 0;
    auto a = [&iii]() {
        std::cout << "direct function call: " << ++iii << std::endl;
    };
    typedef decltype(a) a_type;
    Function<a_type> directCaller(a);
    directCaller.Do();
    Function<a_type> directCaller2(directCaller);
    directCaller2.Do();
    std::cout << "direct function call: " << ++iii << std::endl;


    // Lambdas can be used as well
    Object thingy;
    thingy.set("lala", 304);
    std::function<void() > thingy_modifier = [&thingy]() {
        int temp = thingy.get<int>("lala");
        thingy.add("lala", ++temp); //add is alias for set
        std::cout << thingy.get<int>("lala") << std::endl;
    };

    //Functions differ from objects in that they are initialized with lambdas
    Function < std::function<void()> > modify(thingy_modifier);

    //A function's lambda can be accessed directly with "Do"
    modify.Do();
    modify.Do();
    thingy.set("modify", thingy_modifier);
    //thingy.get < std::function<void()> >("modify")();
    thingy.get < std::function<void()>>("modify")();
    thingy.add("modify", modify);
    std::cout << "Wow" << std::endl;

    // fexec can be used to call a set Do lambda. Note that fexec does not access the parent 
    thingy.fexec < std::function<void()>>("modify");

    thingy.add("1234", thingy_modifier);
    std::cout << "test-1" << std::endl;
    thingy.lexec < std::function<void()>>("1234");
    std::cout << "testinf" << std::endl;

    //===================================================================================================================
    //Also note that for functions returning non-void, the return type must be a pointer whose contents will be allocated on heap.
    //Example:

    struct ww {

        static int * add(int x, int y) {
            return new int(x + y);
        }
    };
    object.setFunc(ww::add); // sets object's function pointer to the add function.
    x = object.call<int>(5, 6); // returns 11. The dereferenced return type is specified in angle brackets.
    std::cout << x << std::endl;


    //===================================================================================================================
    //Members of type Object can designate a parent and pass searches for variables and function calls to their parent.
    //Example:
    Object child;
    child.my_parent = &object;
    object.my_parent = nullptr;
    std::cout << child.get<int>("x") << std::endl; // gets the member of type int whose name is "x" from object. prints 5.
    std::cout << "child has \"x\": " << child.has("x") << std::endl;
    std::cout << "child has int x: " << child.has<int>("x") << std::endl;
    std::cout << "child has float x: " << child.has<float>("x") << std::endl;
    //===================================================================================================================
    //Direct function calls can also be designated object parent if they are equal to the default value, nullptr, in the child.
    //Example:
    std::cout << child.call<int>(5, 6) << std::endl; // returns 11. The dereferenced return type is specified in angle brackets.
    //===================================================================================================================
    //Indirect function calls can also be performed. Objects which can directly call a function can be placed inside of other objects using the "object.set(std::string name, Type T)" method. An outer object can call an inner object by with the method "object.exec<Return_Type T>(std::string name, Parameter_Pack P)". 
    //Example:
    object.set("child", child); // add a new member child to object and set its name to "child". 
    std::cout << object.exec<int>("child", 5, 6) << std::endl; // tells member whose name is "child" to perform the call function. 
    //===================================================================================================================
    //Note that "Object child" is inside of "Object object" and that "Object child" has access to "Object object". This pattern can also be applied to subclasses of Object. 
    //For Example:

    class Computer : public Object {
    };

    class Printer : public Object {
    };
    Computer comp;
    Printer p;
    p.setFunc(ss::print);
    comp.set("print", p);
    comp.exec("print"); // Computer calls Printer's print function.
    return 0;
}
