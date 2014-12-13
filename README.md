Prototypal_C
============

 Prototypal_C is a header implementing a class that allows users to write dynamic, type-safe, prototypal inheritance based c++ code. The Object class contained within this header can be instantiated to create generic containers capable of augmenting themselves with members and functions of various types. Members are accessed indirectly by passing a string to a "get" function. For example: 

  Object object;
  
  int x = 5;
  
  object.set("x",x);              // add a new member x to object and set its name to "x".
  
  int i = object.get<int>("x");   // get member of type int whose name is "x".
  
  std::cout << i << std::endl;    // print 5.
  
 
===================================================================================================

  
//One member function can be called directly using the "call" method, which avoids the overhead of object storage and retrieval. This feature allows members of type Object to have their own persistent variables.

  struct ss{static void print() {std::cout << "hello world" << std::endl;} };
  
  object.setFunc(ss::print);  // sets object's function pointer to the print function.
  
  object.call();                  // directly calls the print function. 
  
 
===================================================================================================

  
//Note that only static global functions, non-static global functions, and non-static class member functions can be passed using setFunc() and call(). Member functions setFunc() and call() can also be used to pass parameters of primitive types or pointers to class types. 


  struct vv {static void func(Object * o, int x) {} };
  
  object.setFunc(vv::func );    // sets object's function pointer to the func function.
  
  Object * ob = &object;
  
  object.call(ob, x);        // directly calls the func function.
  
 
===================================================================================================

  
//Also note that for functions returning non-void, the return type must be a pointer whose contents will be allocated on heap. The object will free the memory internally.

  struct ww {static int * add(int x, int y) {return new int(x+y);} };
  
  object.setFunc(ww::add);      // sets object's function pointer to the add function.
  
  x = object.call<int>(5, 6);     // returns 11. The dereferenced return type is specified in angle brackets.
  
  std::cout << x << std::endl;
  
 
===================================================================================================

  
//Members of type Object can designate a parent and pass searches for variables and function calls to their parent. For example:

  Object child;
  
  child.my_parent = &object;
  
  object.my_parent = nullptr;
  
  std::cout << object.get<int>("x") << std::endl;           // gets the member of type int whose name is "x" from object. prints 5.
  
//  Standard function lambdas can be used to call functons too:
      
    Object thingy;
    
    thingy.set("lala", 304);
    
    std::function<void() > thingy_modifier = [&thingy]()
    {
    
        int temp = thingy.get<int>("lala");
        
        thingy.add("lala", ++temp); // add is alias for set
        
        std::cout << thingy.get<int>("lala") << std::endl;
        
    };
    
    thingy.set("modify", thingy_modifier);
    
    thingy.get < std::function<void()>>("modify")(); // print 305

    //  The lexec function can be used to call a standard lamnda function.
    
    //  Note that function parameters can go after the string name of the function.
    
    thingy.lexec < std::function<void()>>("modify"); // print 306
    
===================================================================================================

// The "has" function can be used to check if an object or its parent has an element

    std::cout << child.has("x") << std::endl; // Tyue
    
    std::cout << "child has int x: " << child.has<int>("x") << std::endl; //True
    
    std::cout << "child has float x: " << child.has<float>("x") << std::endl; //False.
    
    //The above line is false because child does not have float x.
    
===================================================================================================

  
// Direct function calls can also be passed up to the object's parent if the function call wasn't set in the child.

  std::cout << child.call<int>(5, 6) << std::endl;         // returns 11. The dereferenced return type is specified in angle brackets.
  
 
===================================================================================================

  
//Indirect function calls can also be performed. Objects which can directly call a function can be placed inside of other objects using the "object.set(std::string name, Type T)" method. An outer object can call an inner object by with the method "object.Do<Return_Type T>(std::string name, Parameter_Pack P)". 

  object.set("child", child);     // add a new member child to object and set its name to "child". 
  
  std::cout << object.Do<int>("child", 5, 6) << std::endl;  // tells member whose name is "child" to perform the call function. 
  
 
===================================================================================================

  
  //Note that "Object child" is inside of "Object object" and that "Object child" has access to "Object object". This pattern can also be applied to subclasses of Object. 

  class Computer : public Object {};
  
  class Printer : public Object {};
  
  Computer comp;
  
  Printer p;
  
  p.setFunc(ss::print);
  
  comp.set("print",p);
  
  comp.Do("print");               // Computer calls Printer's print function.
  
 
===================================================================================================

  
 In conclusion, by using the Prototypal_C header with the above functions and design patterns, c++ programmers can implement various design patterns and programming techniques that are not readily availible in the language. 
