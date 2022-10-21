#include <vsg/all.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include <vsgXchange/Version.h>
#include <vsgXchange/all.h>

void print(const vsg::ShaderSet& shaderSet, std::ostream& out)
{
    out<<"stages.size() = "<<shaderSet.stages.size()<<std::endl;
    for(auto& shaderStage : shaderSet.stages)
    {
        out<<"  ShaderStage {"<<std::endl;
        out<<"    flags = "<<shaderStage->flags<<std::endl;
        out<<"    stage = "<<shaderStage->stage<<std::endl;
        out<<"    entryPointName = "<<shaderStage->entryPointName<<std::endl;
        out<<"    module = "<<shaderStage->module<<std::endl;
        std::cout<<"  }"<<std::endl;
    }

    out<<std::endl;
    out<<"attributeBindings.size() = "<<shaderSet.attributeBindings.size()<<std::endl;
    for(auto& ab : shaderSet.attributeBindings)
    {
        out<<"  AttributeBinding {"<<std::endl;
        out<<"    name = "<<ab.name<<std::endl;
        out<<"    define = "<<ab.define<<std::endl;
        out<<"    location = "<<ab.location<<std::endl;
        out<<"    format = "<<ab.format<<std::endl;
        out<<"    data = "<<ab.data<<std::endl;
        out<<"  }"<<std::endl;
    }

    out<<std::endl;
    out<<"uniformBindings.size() = "<<shaderSet.uniformBindings.size()<<std::endl;
    for(auto& ub : shaderSet.uniformBindings)
    {
        out<<"  UniformBinding {"<<std::endl;
        out<<"    name = "<<ub.name<<std::endl;
        out<<"    define = "<<ub.define<<std::endl;
        out<<"    set = "<<ub.set<<std::endl;
        out<<"    binding = "<<ub.binding<<std::endl;
        out<<"    descriptorType = "<<ub.descriptorType<<std::endl;
        out<<"    stageFlags = "<<ub.stageFlags<<std::endl;
        out<<"    data = "<<ub.data<<std::endl;
        out<<"  }"<<std::endl;
    }

    out<<std::endl;
    out<<"pushConstantRanges.size() = "<<shaderSet.pushConstantRanges.size()<<std::endl;
    for(auto& pcr : shaderSet.pushConstantRanges)
    {
        out<<"  PushConstantRange {"<<std::endl;
        out<<"    name = "<<pcr.name<<std::endl;
        out<<"    define = "<<pcr.define<<std::endl;
        out<<"    range = { stageFlags = "<<pcr.range.stageFlags<<", offset = "<<pcr.range.offset<<", size = "<<pcr.range.size<<" }"<<std::endl;;
        out<<"  }"<<std::endl;
    }
    out<<std::endl;
    out<<"definesArrayStates.size() = "<<shaderSet.definesArrayStates.size()<<std::endl;
    for(auto& das : shaderSet.definesArrayStates)
    {
        out<<"  DefinesArrayState {"<<std::endl;
        out<<"    defines = { ";
        for(auto& define : das.defines) out<<define<<" ";
        out<<"}"<<std::endl;
        out<<"    arrayState = "<<das.arrayState<<std::endl;
        out<<"  }"<<std::endl;
    }

    out<<std::endl;
    out<<"optionalDefines.size() = "<<shaderSet.optionalDefines.size()<<std::endl;
    for(auto& define : shaderSet.optionalDefines)
    {
        out<<"    define = "<<define<<std::endl;
    }

    out<<std::endl;
    out<<"variants.size() = "<<shaderSet.variants.size()<<std::endl;
    for(auto& [shaderCompileSettings, shaderStages] : shaderSet.variants)
    {
        out<<"  Varient {"<<std::endl;

        out<<"    shaderCompileSettings = "<<shaderCompileSettings<<" : defines ";
        for(auto& define : shaderCompileSettings->defines) out<<define<<" ";
        out<<std::endl;

        out<<"    shaderStages = "<<shaderStages.size()<<std::endl;
        out<<"  }"<<std::endl;
    }

#if 0
        std::vector<DefinesArrayState> definesArrayStates; // put more constrained ArrayState matches first so they are matched first.
#endif
}

std::set<std::string> supportedDefines(const vsg::ShaderSet& shaderSet)
{
    std::set<std::string> defines;

    for(auto& ab : shaderSet.attributeBindings)
    {
        if (!ab.define.empty()) defines.insert(ab.define);
    }

    for(auto& ub : shaderSet.uniformBindings)
    {
        if (!ub.define.empty()) defines.insert(ub.define);
    }

    for(auto& pcr : shaderSet.pushConstantRanges)
    {
        if (!pcr.define.empty()) defines.insert(pcr.define);
    }

    for(auto& define : shaderSet.optionalDefines)
    {
        defines.insert(define);
    }

    return defines;
}

int main(int argc, char** argv)
{
    // use the vsg::Options object to pass the ReaderWriter_all to use when reading files.
    auto options = vsg::Options::create(vsgXchange::all::create());
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();

    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    // read any command line options that the ReaderWrite support
    arguments.read(options);
    if (argc <= 1) return 0;

    auto inputFilename = arguments.value<vsg::Path>("", "-i");
    std::cout<<"inputFilename = "<<inputFilename<<std::endl;

    auto outputFilename = arguments.value<vsg::Path>("", "-o");
    std::cout<<"outputFilename = "<<outputFilename<<std::endl;

    bool stripShaderSetBeforeWrite = arguments.read({"-s", "--strip"});

    vsg::ref_ptr<vsg::ShaderSet> shaderSet;
    if (arguments.read("--text")) { options->shaderSets["text"] = shaderSet = vsg::createTextShaderSet(options); }
    if (arguments.read("--flat")) { options->shaderSets["flat"] = shaderSet = vsg::createFlatShadedShaderSet(options); }
    if (arguments.read("--phong")) { options->shaderSets["phong"] = shaderSet = vsg::createPhongShaderSet(options); }
    if (arguments.read("--pbr")) { options->shaderSets["pbr"] = shaderSet = vsg::createPhysicsBasedRenderingShaderSet(options); }

    if (inputFilename)
    {
        auto object = vsg::read(inputFilename, options);
        if (!object)
        {
            std::cout<<"Unable to load "<<inputFilename<<std::endl;
            return 1;
        }

        shaderSet = object.cast<vsg::ShaderSet>();
        if (!shaderSet)
        {
            std::cout<<"Loaded file is not a ShaderSet"<<std::endl;
            return 1;
        }
    }

    std::cout<<"shaderSet = "<<shaderSet<<std::endl;

    if (!shaderSet)
    {
        std::cout<<"No vsg::ShaderSet to process."<<std::endl;
        return 1;
    }

    // get the defines supported by the ShaderSet
    auto defines = supportedDefines(*shaderSet);

    std::cout<<"\nSupported defines.size() = "<<defines.size()<<std::endl;
    for(auto& define : defines)
    {
        std::cout<<"   "<<define<<std::endl;
    }

    std::string str;
    while(arguments.read({"-v", "--variant"}, str))
    {
        std::cout<<"varient : "<<str<<std::endl;
        auto scs = vsg::ShaderCompileSettings::create();

        std::cout<<"   ";
        std::stringstream sstr(str);
        while(sstr)
        {
            std::string s;
            sstr >> s;
            if (!s.empty())
            {
                if (defines.count(s)==0)
                {
                    std::cout<<"variant define [ "<<s<<" ] not supported"<<std::endl;
                    return 1;
                }

                std::cout<<s<<", ";
                scs->defines.insert(s);
            }
        }

        // assign the variant
        shaderSet->getShaderStages(scs);

        std::cout<<std::endl;
    }

    // load remain command line parameters as models to help fill out the required ShaderSet variants
    for(int i = 1; i<argc; ++i)
    {
        vsg::Path filename(argv[i]);
        if (auto model = vsg::read(filename, options))
        {
            std::cout<<"Loaded filename = "<<filename<<", model = "<<model<<std::endl;
        }
    }

    // print out details of the ShaderSet
    print(*shaderSet, std::cout);

    // keep track of the all the ShaderStages and share any that are the same
    std::vector<vsg::ref_ptr<vsg::ShaderStage>> existing_stages;

    auto shaderCompiler = vsg::ShaderCompiler::create();
    if (shaderCompiler->supported())
    {
        std::cout<<"\ncompiling shaderSet->variants.size() = "<<shaderSet->variants.size()<<std::endl;
        std::cout<<"{"<<std::endl;
        for(auto& [shaderCompileSetting, stagesToCompile] : shaderSet->variants)
        {
            std::cout<<"    "<<shaderCompileSetting<<" : ";
            for(auto& define : shaderCompileSetting->defines) std::cout<<define<<" ";

            shaderCompiler->compile(stagesToCompile, {}, options);
            for(auto& stage : stagesToCompile)
            {
                stage->module->source.clear();
            }

            for(auto& stage : stagesToCompile)
            {
                vsg::ref_ptr<vsg::ShaderStage> match;
                for(auto& existing_stage : existing_stages)
                {
                    bool module_matched = stage->module->code == existing_stage->module->code;
                    if (module_matched)
                    {
                        // share the matched ShaderModule
                        stage->module = existing_stage->module;

                        if (vsg::compare_pointer(stage, existing_stage)==0)
                        {
                            match = existing_stage;
                        }
                        break;
                    }
                }
                if (match)
                {
                    // share the whole ShaderStage
                    stage = match;
                }
                else
                {
                    existing_stages.push_back(stage);
                }
            }

            std::cout<<std::endl;
        }
        std::cout<<"}"<<std::endl;
    }

    std::cout<<"stages.size() = "<<existing_stages.size()<<std::endl;
    for(auto& stage : existing_stages)
    {
        std::cout<<"   "<<stage<<" "<<stage->module<<" "<<stage->module->code.size()<<std::endl;
    }

    if (stripShaderSetBeforeWrite)
    {
        shaderSet->stages.clear();
        shaderSet->attributeBindings.clear();
        shaderSet->uniformBindings.clear();
        shaderSet->pushConstantRanges.clear();
        shaderSet->definesArrayStates.clear();
        shaderSet->optionalDefines.clear();
        shaderSet->defaultGraphicsPipelineStates.clear();
    }

    if (outputFilename) vsg::write(shaderSet, outputFilename, options);

    return 0;
}
