Prototypal_C
============

Prototypal_C is a header implementing a class that allows users to write dynamic, type-safe, prototypal inheritance based c++ code. The Object class contained within this header can be instantiated to create generic containers capable of augmenting themselves with members and functions of various types. Members are accessed indirectly by passing a string to a "get" function. 

Example: 
  Object object;
  int x = 5;
  object.set("x",x);              // add a new member x to object and set its name to "x"
  int i = object.get<int>("x");   // get member of type int whose name is "x". returns 5



One member function can be called directly using the "call" method, which avoids the overhead of object storage and retrieval. This feature allows members of type Object to have their own persistent variables.

Example:
  static void print() {std::cout << "hello world" << std::endl;} 
  object.setFunc("print",print);  // sets object's function pointer to the print function
  object.call();                  // directly calls the print function 



Note that only static global functions, non-static global functions, and non-static class member functions can be passed using setFunc() and call(). Member functions setFunc() and call() can also be used to pass parameters of primitive types or pointers to class types. 

Example:
  void func(Object * o, int x) {} 
  object.setFunc("func",func);    // sets object's function pointer to the func function
  object.call(&object, x);        // directly calls the func function
  
  
  
Also note that for functions returning non-void, the return type must be a pointer whose contents will be allocated on heap.

Example:
  int * add(int x, int y) {return new int(x+y);}
  object.setFunc("add",add);      // sets object's function pointer to the add function
  x = object.call<int>(5, 6);     // returns 11. The dereferenced return type is specified in angle brackets.



Members of type Object can designate a parent and pass searches for variables and function calls to their parent.

Example:
  Object child;
  child.parent = &object;
  object.get<int>("x");           // gets the member of type int whose name is "x" from object. returns 5
  
  
  
Direct function calls can also be designated object parent if they are equal to the default value, nullptr, in the child.

Example:
  child.call<int>(5, 6);          // returns 11. The dereferenced return type is specified in angle brackets.



Indirect function calls can also be performed. Objects which can directly call a function can be placed inside of other objects using the "object.set(std::string name, Type T)" method. An outer object can call an inner object by with the method "object.Do<Return_Type T>(std::string name, Parameter_Pack P)". 

Example:
  object.set("child", child);     // add a new member child to object and set its name to "child" 
  object.Do<int>("child", 5, 6);  // tells member whose name is "child" to perform the call function. 
  
  
  
  Note that "Object child" is inside of "Object object" and that "Object child" has access to "Object object".
