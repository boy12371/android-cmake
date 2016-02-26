/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2016 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraAndroidGradleGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cm_jsoncpp_writer.h"

//----------------------------------------------------------------------------
void cmExtraAndroidGradleGenerator
::GetDocumentation(cmDocumentationEntry& entry, const std::string&) const
{
  entry.Name = GetName();
  entry.Brief = "Generates Android Gradle build files.";
}

cmExtraAndroidGradleGenerator
::cmExtraAndroidGradleGenerator() : cmExternalMakefileProjectGenerator()
{
  SupportedGlobalGenerators.push_back("Unix Makefiles");
  SupportedGlobalGenerators.push_back("Ninja");
}

void cmExtraAndroidGradleGenerator
::Generate()
{
  GenerateProject(this->GlobalGenerator->GetLocalGenerators());
}

void cmExtraAndroidGradleGenerator
::GenerateProject(const cmLocalGenerators &generators)
{
  if (generators.size() == 0)
    return;

  cmMakefile *makefile = generators[0]->GetMakefile();

  std::string output = makefile->GetCurrentBinaryDirectory();
  output += "/android_gradle_build.json";

  cmGeneratedFileStream fileStream{output.c_str()};
  if(!fileStream)
    return;

  Json::Value NativeBuildConfig;

  // cleanCommand
  std::vector<std::string> cleanCommand = {
    makefile->GetRequiredDefinition("CMAKE_COMMAND"),
    "--build", makefile->GetHomeOutputDirectory(),
    "--target", "clean"
  };
  for (const auto &token : cleanCommand)
    NativeBuildConfig["cleanCommand"].append(token);

  // toolchains
  Json::Value toolchains;
  const std::string cCompiler =
    makefile->GetSafeDefinition("CMAKE_C_COMPILER");
  const std::string cppCompiler =
    makefile->GetSafeDefinition("CMAKE_CXX_COMPILER");
  const std::string toolchain =
    std::to_string(std::hash<std::string>{}(cCompiler + '\0' + cppCompiler));
  if (!cCompiler.empty())
    toolchains[toolchain]["cCompilerExecutable"] = cCompiler;
  if (!cppCompiler.empty())
    toolchains[toolchain]["cppCompilerExecutable"] = cppCompiler;
  NativeBuildConfig["toolchains"] = toolchains;

  std::set<std::string> cFileExtensions, cppFileExtensions;

  for (const auto &generator : generators)
  {
    makefile = generator->GetMakefile();

    // buildFiles
    const std::string directory = makefile->GetCurrentSourceDirectory();
    NativeBuildConfig["buildFiles"].append(directory + "/CMakeLists.txt");

    // libraries
    for (const auto &pair: makefile->GetTargets())
    {
      const auto &name = pair.first;
      const auto &target = pair.second;
      switch(target.GetType())
      {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
        case cmTarget::OBJECT_LIBRARY:
          {
            std::string library = name;

            const std::string config =
              makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");

            if (!config.empty())
              library += '-' + config;

            const std::string abi =
              makefile->GetSafeDefinition("ANDROID_ABI");

            if (!abi.empty())
              library += '-' + abi;

            NativeBuildConfig["libraries"][library] =
              ExportTarget(&target, generator, toolchain, config, abi);
            {
              const auto extensions =
                ExportExtensions("C", &target, makefile);
              cFileExtensions.insert(extensions.begin(), extensions.end());
            }
            {
              const auto extensions =
                ExportExtensions("CXX", &target, makefile);
              cppFileExtensions.insert(extensions.begin(), extensions.end());
            }
          }
          break;
        default:
          break;
      }
    }
  }

  // cFileExtensions
  for (const auto extension : cFileExtensions)
    NativeBuildConfig["cFileExtensions"].append(extension);

  // cppFileExtensions
  for (const auto extension : cppFileExtensions)
    NativeBuildConfig["cppFileExtensions"].append(extension);

  fileStream << NativeBuildConfig;
}

std::set<std::string> cmExtraAndroidGradleGenerator
::ExportExtensions(const std::string language,
                   const cmTarget *target,
                   const cmMakefile *makefile)
{
  std::set<std::string> extensions;

  const std::string config = makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");

  std::vector<cmSourceFile *> sources;
  target->GetSourceFiles(sources, config);

  for (const auto &source : sources)
    if (language == source->GetLanguage())
      extensions.insert(source->GetExtension());

  return extensions;
}

Json::Value cmExtraAndroidGradleGenerator
::ExportTarget(const cmTarget *target,
               const cmLocalGenerator *generator,
               const std::string &toolchain,
               const std::string &config,
               const std::string &abi)
{
  Json::Value NativeLibrary;

  // buildCommand
  const cmMakefile *makefile = generator->GetMakefile();
  std::vector<std::string> buildCommand = {
    makefile->GetRequiredDefinition("CMAKE_COMMAND"),
    "--build", makefile->GetHomeOutputDirectory(),
    "--target", target->GetName()
  };
  for (const auto &token : buildCommand)
    NativeLibrary["buildCommand"].append(token);

  // buildType
  if (!config.empty())
    NativeLibrary["buildType"] = config;

  // artifactName
  NativeLibrary["artifactName"] = target->GetName();

  // abi
  if (!abi.empty())
    NativeLibrary["abi"] = abi;

  // output
  if (target->GetType() != cmTarget::OBJECT_LIBRARY) {
    cmGeneratorTarget *gt = this->GlobalGenerator->GetGeneratorTarget(target);
    const std::string output = gt->GetLocation(config);
    if (!output.empty())
      NativeLibrary["output"] = output;
  }

  // toolchain
  NativeLibrary["toolchain"] = toolchain;

  // files
  std::vector<cmSourceFile *> sources;
  target->GetSourceFiles(sources, config);
  for (const auto &source : sources)
  {
    const std::string language = source->GetLanguage();
    if (language == "C" || language == "CXX")
      NativeLibrary["files"].append(ExportSource(target, generator, source));
  }

  return NativeLibrary;
}

Json::Value cmExtraAndroidGradleGenerator
::ExportSource(const cmTarget *target,
               const cmLocalGenerator *generator,
               const cmSourceFile *source)
{
  Json::Value NativeSourceFile;

  // src
  NativeSourceFile["src"] = source->GetFullPath();

  // workingDirectory
  std::string workingDirectory =
    generator->GetMakefile()->GetCurrentBinaryDirectory();
  workingDirectory =
    generator->Convert(workingDirectory, cmLocalGenerator::FULL);
  NativeSourceFile["workingDirectory"] = workingDirectory;

  // flagsString
  cmGeneratorTarget *gt = this->GlobalGenerator->GetGeneratorTarget(target);
  cmAndroidGradleTargetGenerator tg(gt);
  NativeSourceFile["flags"] = tg.ExportFlags(source);

  return NativeSourceFile;
}

void cmExtraAndroidGradleGenerator::cmAndroidGradleTargetGenerator
::AddIncludeFlags(std::string &flags, const std::string &language)
{
  const std::string config =
    this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              language, config);
  const std::string includeFlags =
    this->LocalGenerator->GetIncludeFlags(includes, this->GeneratorTarget,
                                          language, false, false, config);
  this->LocalGenerator->AppendFlags(flags, includeFlags);
}

// Copied from cmMakefileTargetGenerator::WriteObjectBuildFile.
std::string cmExtraAndroidGradleGenerator::cmAndroidGradleTargetGenerator
::ExportFlags(const cmSourceFile *source)
{
  const std::string language = source->GetLanguage();

  // Build the set of compiler flags.
  std::string flags;

  // Add language-specific flags.
  const std::string languageFlags = "$(" + language + "_FLAGS)";
  this->LocalGenerator->AppendFlags(flags, languageFlags);

  const std::string config =
    cmSystemTools::UpperCase(this->LocalGenerator->GetConfigName());

  // Add flags from source file properties.
  if (const char *compileFlags = source->GetProperty("COMPILE_FLAGS"))
    this->LocalGenerator->AppendFlags(flags, compileFlags);

  // Add language-specific defines.
  std::set<std::string> defines;

  // Add source-sepcific preprocessor definitions.
  if(const char* compileDefinitions =
       source->GetProperty("COMPILE_DEFINITIONS"))
    this->LocalGenerator->AppendDefines(defines, compileDefinitions);
  if(const char *compileDefinitions =
       source->GetProperty("COMPILE_DEFINITIONS_" + config))
    this->LocalGenerator->AppendDefines(defines, compileDefinitions);

  const std::string languageDefines = "$(" + language + "_DEFINES)";
  std::string definesString = languageDefines;
  this->LocalGenerator->JoinDefines(defines, definesString, language);

  const std::string languageIncludes = "$(" + language + "_INCLUDES)";

  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_COMPILE";
  vars.CMTarget = Target;
  vars.Language = language.c_str();
  vars.Flags = flags.c_str();
  vars.Defines = definesString.c_str();
  vars.Includes = languageIncludes.c_str();

  // Construct the compile rules.
  const std::string compileRuleVar = "CMAKE_" + language + "_COMPILE_OBJECT";
  std::string compileCommand =
    this->Makefile->GetRequiredDefinition(compileRuleVar);

  const auto &unusedRules = {
    "<CMAKE_C_COMPILER>", "<CMAKE_CXX_COMPILER>",
    "-c", "-o", "<OBJECT>", "<SOURCE>"
  };
  // Remove unwanted rules.
  for (const std::string &unusedRule : unusedRules)
  {
    const auto &position = compileCommand.find(unusedRule);
    if (position != std::string::npos)
      compileCommand.erase(position, unusedRule.size());
  }

  this->LocalGenerator->ExpandRuleVariables(compileCommand, vars);
  compileCommand.replace(compileCommand.find(languageFlags),
                         languageFlags.size(), GetFlags(language));
  compileCommand.replace(compileCommand.find(languageDefines),
                         languageDefines.size(), GetDefines(language));
  compileCommand.replace(compileCommand.find(languageIncludes),
                         languageIncludes.size(), GetIncludes(language));

  return compileCommand;
}
