
shader.vert file
#version 450

layout(location = 0) in vec3 inColor; // Declare input for color
layout(location = 0) out vec3 fragColor; // Declare output for color

// Correctly define the array of positions
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    // Use positions array with gl_VertexIndex
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = inColor; // Pass the input color to the fragment shader
}


shader.frag file
#version 450



layout (location = 0 ) out vec4 outColor;

void main(){
     outColor = vec4(1.0, 0.0, 0.0, 1.0);
}


compile.bat file
glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv

ikPipeline.hpp file

#ifndef IKPIPELINE_HPP
#define IKPIPELINE_HPP

#include <string>
#include <vector>

namespace ikE {

    class ikePipeline {
    public:
        ikePipeline(const std::string& vertFilepath, const std::string& fragFilepath);


    private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);
    };
}



#endif 

ikPipeline.cpp file

#include "ikPipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>


namespace ikE {

    ikePipeline::ikePipeline(const std::string& vertFilepath, const std::string& fragFilepath) {
        createGraphicsPipeline(vertFilepath, fragFilepath);
    }

     std::vector<char> ikePipeline::readFile(const std::string& filepath) {


        std::ifstream file(filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file" + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }
    void ikePipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath) {
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        std::cout << "Vertex Shader Code size:" << vertCode.size() << "\n";
        std::cout << "Fragment Shader Code size:" << fragCode.size() << "\n";


    }
}//namespace





First_App.hpp

#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikWindow.hpp"
#include "ikPipeline.hpp"
#include "ikDeviceEngine.hpp"
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run();

	private:
		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
        ikePipeline ikePipeline{"frag.spv","vert.spv"};
	};

} //namepace
#endif //header guard

