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
