##Graphics Pipeline
Summary: 
        the idea is that because gpu's are multithreaded meaning it can compute multiple threads simutaneously
        which cpu's can not  
        and this merit of the gpu also comes with its flaws meaning we can not write code the same way for gpu's as for cpu's where we have complete freedom
        GPU's have programmable stages and fixed stages 
        the programmabel stage is called shaders:
                                   * vertex shader: processes each vertex individualy and perform transformation i.e rotation and translation
                                   * fragment shader: processes each fragment individually and ouputs values such as the color by using interpolated data from things 
                                     like textures,normals and lighting


###we will create the vertex
###  NOTE: we can create a new folder and store all the shader code if we want 
        but we have to tell the program the path the folder is in
### shader.vert file
   [] we need the specify the glsl version that we are using 
      i.e #version 450 which corresponds to the glsl version 1.5    

#[] we will HARDCODE the input assembley stage by using vec2 so that we can feed the vertex shader with informations
      * vec2 is a built in type in the glsl programe that contain 2 floating point values
      * we will initialize a position array of length 3. one for each corner of the traingle
          use what ever value that is in the range of -1 to 1
          this six values will end up looking like this i.e 
          vec2 positions[3] = vec2{
          vec2(0.0,-0,5),
          vec2(0.5,0.5),
          vec2(-0.5,0.5)
          };

 ==Visual Representation==
                             X
topleft(-1.-1)____________________________(1,-1)
             |                             |
             |            0                |
             |          /   \              |
             |         /     \             |
             |        /       \            |
             |       / (0.0)   \           |
         Y   |      /     *     \          |
             |     /_____________\         |
             |    2               3        |
        (-1,1)_____________________________(1,1)Bottomright        

  *NOTE HARDCODING the input assembly values in the vert.shader is not a good idea we usually use vertex buffer to pass such information
        but we will use it for now          
   []  we create the main fucntion void main(){} which will be executed once for each vertex that we have
       thinking back at the idea of the graphicsPipeline 
       as input our vertex shader will get each vertex as input from the input assembler stage
       and needs to output a position rather than doing this in the return of our main() we will leave it as void 
       Instead we will use a special variable in  glsl called gl_Position that we assign a value to it,to act as our output
       
   [] inside the main()
     * we assign value to gl_Position which is a 4dimensional vector that maps to the frame buffer output image
        where the topleft corner is (-1,-1) and the bottomright to (1,1) meaning the center is at (0.0)
        this is the information that we have to submit to the frament shader after we are done in the main()        
                  X
topleft(-1.-1)___________(1,-1)
             |           |
             |  (0.0)    |
         Y   |    *      |
             |           |
             |___________|
        (-1,1)            (1,1)bottomright
    
       *Example:
               gl_Position = vec4(position   [gl_VertexIndex])       ,0.0,                              1.0);
                              ^                   ^                    ^                                 ^
                              |                   |                    |                                 |
                    is a vec4 where             we can index         this is the                this is the normalize device
                    the X and Y comes           into it.             Z value which              cordinate that divides the
                    from the positions          gl_VertexIndex       is like having             vector list by it's last component
                    array                       which contains       a layer where Z            meaning we are dividing everything 
                                                the index of         equals 0 or the            by 1.0
                                                the current vertex   represent the outmost 
                                                for each time our    layer. it can range 
                                                Main() is run        from 0 to 1

                                                
  ###Now we will create the fragment shader file

##shader.frag file

# [] Unlike the shader.vert that has a built in output fragment shader has none so we need to declare that ourself
   Example:
           layout         (location = 0)                   out                   vec4            outColor
             ^                   ^                          ^                     ^                 ^
             |                   |                          |                     |                 |
        we have the           a fragment              next we specify           the return      the variable
        layout qualifier      is capable of           that this variable        type of the     name
        which takes -->       outputing to            should be used as         variable
        a location value      multiple locations      an output with the
                              here we are only        help of the out 
                              using location 0        qualifier


# [] Inside the main() function 
    * we declare and assign values to the outColor variable
      * the value of each component assigned to the outColor variable needs to be between 0(min value) --> 1(max value) range
      *Example;
               outColor = vec4            (1.0,      0.0,          0.0,        1.0);
                            ^               ^         ^             ^           ^
                            |               |         |             |           |
                       we are using        red       green         blue       alpha   
                       a 4 component      channel    channel      channel    channel
                       vector             max value  min value   min value   max value 
                                                                             so to be fully
                                                                             OPAQUE in color

      NOTE: do not think that we are painting the entire image red. instead understand that 
             the fragment shader runs on a per fragment bases which are detairmine by the pixels
             our geometry mostly contain during the rastarization stage

TODO start
Similar as c++ or c programs needs to compile we need to compile our vert and frag shader code into
 Standard portable intermediate representabtion V (SPIR-V)
 []in windows Operating-System
   *navigate to where you installed the VULKANSDK folder->locate the Bin folder -> look for glslc extension
    if the extensions are there Copy the file path
TODO end
 
##we will create the compile.bat file 
  and  we will paste the file path inside 
  make sure the glsc is the last token of the path or present\then we include the name of the shader or if the shader is inside
  a file\then the shader 
  -o to specify the output
   then spv as the extension of both output
  Example:
         glslc shader.vert -o vert.spv
         glslc shader.frag -o frag.spv
TODO start
# save the compile.bat file and compile it through the terminal
  we will then get the spv files after compilation
Overall Behavior
Overall Behavior
When an ikePipeline object is created, its constructor:

Reads the vertex and fragment shader files using readFile.
Logs the sizes of the loaded files.
If any file fails to open, the program throws a std::runtime_error.

The output shows the sizes of the loaded vertex and fragment shaders, ensuring that the files are successfully read.

When an ikePipeline object is created, its constructor:

Reads the vertex and fragment shader files using readFile.
Logs the sizes of the loaded files.
If any file fails to open, the program throws a std::runtime_error.

The output shows the sizes of the loaded vertex and fragment shaders, ensuring that the files are successfully read.

  NOTE: on a mac you might encounter an error the fix is 
        to install the spri-v compiler through home brew
        i.e  run brew install glslang
TODO end

##Now that we have the .spv shader codes we need to read this files into our program
##we need to create  two files a .hpp and .cpp files to hold the graphics pipeline intact

###lets start by creating the hpp file
  
##Overall Behavior
When an ikePipeline object is created, its constructor:

 1. Reads the vertex and fragment shader files using readFile.
 2. Logs the sizes of the loaded files.
 3. If any file fails to open, the program throws a std::runtime_error.

The output shows the sizes of the loaded vertex and fragment shaders, ensuring that the files are successfully read.

###ikpipeline.hpp file
  [] we create the header guards block ifndef,define and endif 
  [] inside the block we will  use the Ike namespace
  [] inside the Ike namespace block we will create a new class ikePipeline
     * that has a private and a public section
  [] inside the ikePipeline class 
Public section
      * we will initialize the constructor with argumentst
      Example:
             ikePipeline(const std::string& vertFilepath,const std::string& fragFilepath)
             
        NOTE: do not forget to include<string> header
Private section
## we need to Declare 2 member functions
 * a readFile method  that takes a const std::string reference to a filepath as an argument and returns a vector container with a char returntype with a static modifier keyword
   Example:
          static std::vector<char> readFile(const std::string& filepath);
          NOTE: don not forget to include<vector> header 
* a helper function void createGraphicsPipeline which takes exact same argument as our constructor
    Example:
           void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath)
            NOTE: it takes             const string ref --> vertfilepath, const stringref --> fragFilepath
            


## lets add an implementation for the createGraphicsPipelin by creating the .cpp file and Defining the functions
##we create ikPipeline.cpp file
  [] inside the file we will include the ikPipeline.hpp file
  [] we will use the ikE namespace
  [] inside the namespace we will grab the readFile() method from hpp --> cpp file
     and define it
     * do not forget to add and scope the class ikePipeline
       Example:
              std::vector<char> ikPipeline::readFile(const std::string& filepath){}
                    ^             ^        ^     ^              ^                 ^
                    |             |        |     |              |                 |
                container      class    scope
                                      operator  method        argurment      method Code
                                                                               Block 
       NOTE: do not forget to include<fstream> header file                                                                               
       PURPOSE: Reads the contents of a file and returns its binary data in a std::vector<char>

       [] inside the readFile method block
      
         * we will initalize a new file stream variable that takes the filepath as argument and some bit flags
             Example:
                     std::ifstream file(filepath, std::ios::ate | std::ios::binary);
                                          ^                ^                  ^
                                          |                |                  |
                                        the file    Opens the file        Opens the file in       
                                        to read     and positions         A binary mode  
                                                    the file pointer      To avoid any text Transformation 
                                                    at the end.           from occuring
                                                    This is used to 
                                                    Determine the size
      
        * we will the create an if statement to know if  our file is not-open  via file.is_open() function
        * then throw a std::runtime error
           Example:
                  if(!file.is_open()){
                     throw std::errow ("failed to open" + filepath);}
                                           ^            ^     ^
                                           |            |     |
                                    the error message  plus the filepath 
                      NOTE: do not forget to inlcude<stdexcept>

                  [] inside the if-block
                        * now we need to the size of the file
                  Example:
                          size_t fileSize = static_cast<size_t>(file.tellg());
                                                                       ^
                                                                       |
                                                                   because we 
                                                                   use ios::ate 
                                                                  bit flag to get
                                                                  to the end of
                                                                  the file we will
                                                                  use tellg()
                                                                  to get the last
                                                                  position which 
                                                                  is the file size
                         * next we need to create a buffer method with a vector container that has the fileSize as it's argument and char as it's returntype
                          Example:
                                  std::vector<char> buffer(fileSize);
                         * because we want to read the data we need to call the seekg(0) method instructing to start from the start of the file
                           Example:
                                   file.seekg(0);
                         * then we call the read the data method and the number of bytes is determined by the fileSize that we want to read
                            Example: 
                                    file.read(buffer.data(), fileSize);
                        * then we close and return the buffer after finish reading
                            Example:
                                    file.close();
                                    return buffer;


## we exit the if-block and create the grahicsPipeline method

TODO start
  similarly we need to go to the .hpp , copy the create graphicsPipeline definition into the .cpp file
TODO end
 
- []inside void ikPipelime::createGraphicsPipeline() method
  * we will need to do is read the vertex and fregment  code we will comeback to it later but for now 
    Example:
          * auto vertCode = readFile(vertFilepath);
          * auto fragCode = readFile(fragFilepath);
  * to be sure that the files are read we will std::cout both files
    Example:
           std::cout << "vertex shader Code size" << vertCode.size() << '\n';
           std::cout << "Frag shader code size" << fragCode.size() << '\n';
           NOTE: do not forget to include<iostream>

- [] we can now at the top create our constructor by taking it's declaration from the .hpp file
   * inside the constructor we call the createGraphicsPipelin() method with vertFilepath and fragFilepath as argument
     Example:                           
             ikPipeline::ikPipeline(const std::string& vertFilepath, const std::string& fragFilepath){
                    createGraphicsPipeline(vertFilepath,fragFilepath);
             }
              
             


##First_App.hpp file
[] we will include"ikPipeline.hpp" header file
Private section  
  * under the instantiated ikWindow class, we will instantiate ikPipeline class and the path to were the vert and frag shader is located as argument
    Example:
           ikPipeline ikPipeline{'vert.spv,frag.spv'};
                           
save and compile 
OUTPUT: will be the sizes of frag and vert shaders 










