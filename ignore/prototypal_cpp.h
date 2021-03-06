#ifndef PROTOTYPAL_CPP
#define PROTOTYPAL_CPP
/*
Copyright [2014] [John-Michael Reed]
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http:// www.apache.org/licenses/LICENSE-2.0
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

#include <typeindex>      //  std::type_index wrapper for type_info
#include <iostream>
#include <unordered_map>
#include <functional>  // For bad_function_call
#include <memory>  //  shared_ptr, typeid

/** type pcast produces a function that takes in an arbitrary # of args and
 *  returns a void pointer. 
 */
typedef void * (*pcast) (...);

/** 
 * Magic number used for type identification 
 */
#define ____OBJECT_TYPE -9223372036854775807LL

/** Dynamic object which is capable of adding static function pointers, 
 *  std::function lambads, and values to itself
 */
class Object
{
    /** 
     * Magic number used for type identification 
     */
    const int64_t my_type = ____OBJECT_TYPE;

    /** Stores a pointer to an object of arbitary type and a typeindex(typeid) 
     *  corresponding to the stored object. 
     */
    struct Shared_Pointer_And_Type
    {
        std::shared_ptr<void> p;
        std::type_index t;

        Shared_Pointer_And_Type() : p(nullptr), t(typeid(void))
        {
        }

        Shared_Pointer_And_Type(const std::shared_ptr<void>pp,
                                const std::type_index tt) : p(pp), t(tt)
        {
        }

        Shared_Pointer_And_Type(const Shared_Pointer_And_Type &other) :
        p(other.p), t(other.t)
        {
        }

        Shared_Pointer_And_Type& operator =
                (const Shared_Pointer_And_Type &other)
        {
            this->p = other.p;
            this->t = other.t;
            return *this;
        }
    };
    /** 
     *  Stores persistent variables, std::function types, and Objects 
     */
    std::unordered_map<std::string, Object::Shared_Pointer_And_Type>
    my_contents;
    /** Re-assignable function pointer.
     *  Set with Object::setFunc and called with Object::call<Return_Type>.
     */
    pcast execute_me;
    /** 
     * Re-assignable pointer to parent of this object.
     * Function in this class will use this pointer to go 
     * up the inheritance hierarchy.
     */
    Object * my_parent;
public:

    /** 
     *  \brief Empty default constructor.
     */
    Object() : my_contents(), execute_me(nullptr), my_parent(nullptr)
    {
    }

    /** 
     *  \brief Standard copy constructor.
     */
    Object(const Object &o) : my_contents(o.my_contents),
    execute_me(o.execute_me), my_parent(o.my_parent)
    {
    }
public:

    /** 
     *  \brief Empty virtual destructor. To be overloaded by derived classes.
     */
    virtual ~Object()
    {
    }

    /** 
     *  \brief Sets the parent of this Object to another Object
     *  @param other_object - new parent
     */
    inline void setParent(Object &other_object)
    {
        if (&other_object != this)
            this->my_parent = &other_object;
        else
        {
            printf("In Object.setParent, Object is not allowed to set its "
                   "parent pointer to itself.\n  "
                   "See line number %d in file %s\n", __LINE__, __FILE__);
            return;
        }
    }

    /** 
     *  \brief Standard assignment operator
     */
    Object& operator =(const Object &other)
    {
        this->my_contents = other.my_contents;
        this->my_parent = other.my_parent;
        this->execute_me = other.execute_me;
        return *this;
    }

    /** 
     *  \brief Passes hashtable contents from one Object to another
     */
    inline void pass_contents(const Object &other)
    {
        this->my_contents = other.my_contents;
    }

    /**
     * \brief Sets function pointer execute_me to the address of a static function. 
     * @param function_pointer - a generic 64-bit function pointer
     */
    template <class Type> void setFunc(Type function_pointer)
    {
        // Prevents the user from setting function pointer to an object
        if (function_pointer == nullptr || (sizeof (function_pointer) !=
                sizeof (this->execute_me)))
        {
            printf("In Object.setFunc, function pointer is null or function "
                   "cannot safely be assigned.\n  "
                   "See line number %d in file %s\n", __LINE__, __FILE__);
            throw std::bad_function_call();
            return;
        }
        else
        {
            this->execute_me = (pcast) function_pointer;
            return;
        }
    }

    /**
     * \brief Add a single object property to the properties hash table with key 
     * string::name and generic value. 
     * Will leak memory if you pass in new int[] allocate array.
     * @param name - name that will be used to retrieve value
     * @param value - a generic value to be added
     */
    template <class Type> void set(const std::string &name, const Type &value)
    {
        std::shared_ptr<Type> shared_pointer = std::make_shared<Type>(value);
        Shared_Pointer_And_Type temp(std::static_pointer_cast<void>
                                     (shared_pointer),
                                     std::type_index(typeid (value)));
        this->my_contents[name] = temp;
        return;
    }

    /** 
     *  \brief Alias for Object.set
     */
    template <typename... Args>
    auto add(Args&&... args) -> decltype(set(std::forward<Args>(args)...))
    {
        return set(std::forward<Args>(args)...);
    }

    /**
     * \brief Checks to see if this object has a variable with name value equal to 
     * std::string name.
     * Throws out_of_range exception when name cannot be found
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an 
     * object somewhere in its parent tree.
     */
    bool has(const std::string &name)
    {
        try
        { // If the element exists, get it.
            ((this->my_contents.at(name)));
            return true;
        }// Else check to see if the my_parent has it
        catch (const std::out_of_range& oor)
        {
            if (this->my_parent != nullptr)
            {
                return this->my_parent->has(name);
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * \brief Checks to see if this object has a variable with name value equal to 
     * std::string name and type equal to Element_Type.
     * Throws out_of_range exception when name cannot be found
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an 
     * object somewhere in its parent tree.
     */
    template <class Element_Type> bool has(const std::string &name)
    {
        try
        { // If the element exists, get it.
            Shared_Pointer_And_Type spt = this->my_contents.at(name);
            if (spt.t == std::type_index(typeid (Element_Type)))
            {
                return true;
            }
            else
            {
                return false;
            }
        }// Else check to see if the my_parent has it
        catch (const std::out_of_range& oor)
        {
            if (this->my_parent != nullptr)
            {
                return this->my_parent->has<Element_Type>(name);
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * \brief Retrieves an element from this object with non-void return type
     * Throws out_of_range exception when name cannot be found
     * @param name - string name of the variable that we are searching for
     * @return Return_Type - return type which must be specified in 
     * angle brackets
     */
    template <class Return_Type> Return_Type get(const std::string &name)
    {
        // If the element exists, get it.
        try
        {
            Shared_Pointer_And_Type spt = this->my_contents.at(name);
            if (spt.t == std::type_index(typeid (Return_Type)))
            {
                Return_Type r = *(std::static_pointer_cast<Return_Type>(spt.p));
                return r;
            }
            else
            {
                printf("In Object.get<class Return_Type>, value Return_Type "
                       "does not match up with a retrievable member's type.\n"
                       "  See line number %d in file %s\n",
                       __LINE__, __FILE__);
                throw std::bad_cast();
            }
        }// Else check to see if the my_parent has it
        catch (const std::out_of_range& oor)
        {
            if (this->my_parent != nullptr)
            {
                return this->my_parent->get<Return_Type>(name);
            }
            else
            {
                printf("In Object.get<class Return_Type>(string name), "
                       "parameter name does not correspond to a named element."
                       "\n  See line number %d in file %s\n",
                       __LINE__, __FILE__);
                throw oor;
            }
        }
    }

    /**
     * \brief Directly calls the generic function pointer execute_me if the return 
     * type is void.
     * Throws bad function call if pointer is null. Will leaks memory if used 
     * with a non-void  function.
     * @param Parameters - generic list of function parameters
     */
    template <class ...A> void call(A... Parameters)
    {
        if (this->execute_me != nullptr)
        {
            (this->execute_me(Parameters...));
            return;
        }
        else if (this->my_parent != nullptr)
        {
            this->my_parent->call(Parameters...);
            return;
        }
        else
        {
            printf("In Object.call, function pointer passed to call is nullptr."
                   "\n  See line number %d in file %s\n", __LINE__, __FILE__);
            throw std::bad_function_call();
            return;
        }
    }

    /**
     * \brief Directly calls the generic function pointer execute_me contained within 
     * this object if the return type is non-void
     * Throws bad function call if pointer is null
     * @param Parameters - generic list of generic list of comma delimited 
     * function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template <class Return_Type, class ...A> Return_Type call(A... Parameters)
    {
        if (this->execute_me != nullptr)
        {
            Return_Type * rptr = reinterpret_cast<Return_Type *>
                    (this->execute_me(Parameters...));
            Return_Type ret = *rptr; // copy contents of hash table
            delete rptr;
            return ret; // return the copy.
        }
        else if (this->my_parent != nullptr)
        {
            return this->my_parent->call<Return_Type>(Parameters...);
        }
        else
        {
            printf("In Object.call<class Return_Type>, function pointer passed "
                   "to call is nullptr.\n  See line number %d in file %s\n",
                   __LINE__, __FILE__);
            throw std::bad_function_call();
        }
    }

    /**
     * \brief Executes a function with no return type by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     */
    template<class ...A> void exec
    (const std::string &function_name, A... Parameters)
    {
        try
        {
            Object::Shared_Pointer_And_Type spt = my_contents.at(
                                                                 (std::string) function_name);
            std::shared_ptr<Object> isObject = std::static_pointer_cast<Object>
                    (spt.p);
            if (isObject->my_type != ____OBJECT_TYPE)
            {
                printf("Element referenced by Object.exec is not an Object.\n  "
                       "See line number %d in file %s\n", __LINE__, __FILE__);
                throw std::bad_function_call();
                return;
            }
            // Call the corresponding function
            isObject->call(Parameters...);
            return;
        }// C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor)
        {
            // Check my_parent.
            if (this->my_parent != nullptr)
            {
                this->my_parent->exec(function_name, Parameters...);
                return;
            }// Give up.
            else
            {
                printf("Function pointer referenced by Object.exec cannot be "
                       "found.\n  See line number %d in file %s\n",
                       __LINE__, __FILE__);
                throw oor;
                return;
            }
        }
    }

    /**
     * \brief Executes a value-returning function by its function name 
     * @param function_name - the key name of the function as a std::string
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class Return_Type, class ...A> Return_Type exec
    (const std::string &function_name, A... Parameters)
    {
        try
        {
            Object::Shared_Pointer_And_Type spt = my_contents.at(
                                                                 (std::string) function_name);
            std::shared_ptr<Object> isObject = std::static_pointer_cast<Object>
                    (spt.p);
            if (isObject->my_type != ____OBJECT_TYPE)
            {
                printf("Element referenced by Object.exec<class Return_Type> is"
                       " not an Object.\n  See line number %d in file %s\n",
                       __LINE__, __FILE__);
                throw std::bad_function_call();
            }
            //  Calls the corresponding function.
            return isObject->call<Return_Type>(Parameters...);
        }//  C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor)
        {
            // Check my_parent.
            if (this->my_parent != nullptr)
                return this->my_parent->exec<Return_Type>
                    (function_name, Parameters...);
                // Give up.
            else
            {
                printf("Function pointer referenced by "
                       "Object.exec<class Return_Type> cannot be found.\n  "
                       "See line number %d in file %s\n", __LINE__, __FILE__);
                throw oor;
            }
        }
    }

    /**
     * \brief Executes a standard function by name. Returns whatever the function returns. 
     * @param function_name - the key name of the standard function as a 
     * std::string
     * @param Parameters - generic list of function parameters
     * @param Standard_Function - type of standard function to execute. 
     * Example: std::function<void(int)>
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class Standard_Function, class Return_Type = void, class ...A>
    Return_Type lexec(const std::string &function_name, A... Parameters)
    {
        try
        {
            Object::Shared_Pointer_And_Type spt = my_contents.at(function_name);
            if (std::type_index(typeid (Standard_Function)) != spt.t)
            {
                printf("Wrong standard function type referenced by "
                       "Object.levec<class Standard_Function>.\n  "
                       "See line number %d in file %s\n", __LINE__, __FILE__);
                throw std::bad_function_call();
            }
            Standard_Function isLambda =
                    *(std::static_pointer_cast<Standard_Function>(spt.p));
            return isLambda(Parameters...);
        }
        catch (std::out_of_range e)
        {
            if (this->my_parent != nullptr)
            {
                return lexec<Standard_Function, Return_Type>
                        (function_name, Parameters...);
            }
            else
            {
                throw e;
            }

        }
    }

};
#endif
