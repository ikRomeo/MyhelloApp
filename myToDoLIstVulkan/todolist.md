
##CREATING A WINDOW WITH THE HELP OF GLFW
 ikwindow.hpp file

1. [] we create the header Guards
inside the header Guard
2. [] we create a namespace ikE block   
inside the namespace block
3. [] we create a class IkeWindow{};
because we are using Glfw it is safe to now
  * include the header that tells glfw to include vulkan 
  * include the glfw.h file which is what glfw uses
inside the classs Window
Private section
    [] we will get the GlfwWindow handler via a pointer variable name window
    [] we will declare an initWindow() with not parameter
    [] the initWindow will need a width and a height which most not be modified so we will declare two const int variable 
    [] we will create an std::string which will hold the name of the window
because we are using std::string it is safe to say we need to add the header file

Public section
   [] we will need to declare the constructor and the destructor
      the constructor will have the width,height and name as parameter
      the destructor has no parameter



 ikwindow.cpp file
because we only declare the variables and class in ikewindow.hpp here we will define them
  [] we include ikwindow.hpp
  [] we will use the namespace ikE block
inside the namespace
     [] we will initialize the constructor with variables
        * the width, heigth, the windows name 
	* inside the constructor we will call the initwindow() 
  [] we define the intiwindow function
     # Note: the initwindow() has the return type::the class::then the function
     #       i.e void IkeWindow::initWindow(){};
        * inside the initWindow(),we will
	  * hint to the glfwWindowHint that we do not need opengl api 
	  * hint to the glfwWindowHint not to resize , we will deal with that when the time comes
          * we will call the glfwCreateWindow() with the
	    * WIDTH,HEIGHT,windowName.c_str(),nullptr,nullptr
  [] we will now implement the destructor method of the class IkeWindow, under the constructor
     # inside the IkeWindow::~IkeWindow destructor
       * we will call the glfwWindowdestroy() that takes the window as an argurment
       * we will call glfwTerminate with no argument to terminate the window resource

 ##We now move on to create another  hpp and cpp file that where we can run the vulkan program with all it's handlers

First_App.hpp file
   
  [] we will include the ikwindow.hpp file
  [] we create a header guard
   # inside the header guard
   we will use the same namespace ikE block
# inside the namespace ikE block
  [] we will create the First_App class{}
# inside the First_App class
Public section
    * we will define a static constexpr WIDTH = 800;
    * we will define a static constexpr HEIGHT = 600;
    * we will call the void run() {};
    # we have to use dangling braces for now just to run the code later we will define the run()
##then we move to the private section of the class IkeWindow
Private section
    * we will instantiate the IkeWindow ikeWindow{ WIDTH,HEIGHT, " a string to convey information i.e hello bro"};
   
   
   
# we can now try to see if we have succcessfully created a window by calling it  in Main.cpp 
##NOTE: we can create the First_App.cpp file before the main but in respect of the dangling braces in the void run(){}; will create the Main.cpp file
###Main.cpp file bb
# the idea is to try and catch an error if the glfw window was not created
 [] we will include First_App.hpp header,cstdlib,iostream,stdexcept(for exception handling)
 [] we create the main() and 
    * instantiate the First_App class to app
    * we will create the try error block
      * inside the try block we will call the app.run()
    * we will create the catch error block which has an argument of a variable e that is used to reference the std::exception method we made const 
             example: (const std::exception& e)
             * inside the catch block we call the std::cerr and with output the e.what() to the screen
             * we return EXIT_FAILURE
#we can now run the code 
#if it is successful we will see a blank window else it will print out error
##we can now create the cpp file for First_App.hpp file 
###First_App.cpp file
   [] we include the First_App.hpp header
      * we use the namespace again ikE 
         inside the namespace 
        * we will define the void First::run(){};
        # inside the run()
          * we will use a while-loop block with an argument that as long as the window is open we can poll events 
              *inside the while-loop
                ## example:
                      while(!ikeWindow.shouldClose());{glfwPoolEvents();}
         
###Now we need to go to the ikWindow.hpp file and initialize the shouldClose()
##NOTE the shouldClose function is a bool function that returns the glfwShouldclose() with the window as an argument 
     Example:
            bool shouldClose(){return glfwWindowShouldClose(window);}


###  Now we can go to the First_App.hpp file 
 * and remove the dangling braces in the void run(){} to void run();
     
  
### go to the ikWindow.hpp file 
   * and create a function to delte the copy constructor and assign operator
   




