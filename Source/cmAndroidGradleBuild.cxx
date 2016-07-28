/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2016 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAndroidGradleBuild.h"

#include <string>

#include "cmGeneratedFileStream.h"
#include "cmGlobalCommonGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cm_jsoncpp_writer.h"

//----------------------------------------------------------------------------
void cmAndroidGradleBuild
::ExportProject(const cmGlobalCommonGenerator *globalGenerator)
{
  const auto localGenerators = globalGenerator->GetLocalGenerators();
  if (localGenerators.size() == 0)
    return;

  std::string output =
    globalGenerator->GetCMakeInstance()->GetHomeOutputDirectory();
  output += "/android_gradle_build.json";

  cmGeneratedFileStream fileStream{output.c_str()};
  if(!fileStream)
    return;

  Json::Value NativeBuildConfig;

  std::vector<cmMakefile *> makefiles = globalGenerator->GetMakefiles();
  if (makefiles.empty())
    return;

  cmMakefile *makefile = makefiles[0];

  // cleanCommand
  const std::string cmakeCommand = cmSystemTools::ConvertToOutputPath(
    makefile->GetRequiredDefinition("CMAKE_COMMAND"));
  const std::string homeOutputDirectory = cmSystemTools::ConvertToOutputPath(
    makefile->GetHomeOutputDirectory());
  std::string cleanCommand = cmakeCommand +
    " --build " + homeOutputDirectory + " --target " + "clean";
  NativeBuildConfig["cleanCommands"].append(cleanCommand);

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

  for (const auto &localGenerator : localGenerators)
  {
    makefile = localGenerator->GetMakefile();

    // buildFiles
    const std::string directory = makefile->GetCurrentSourceDirectory();
    NativeBuildConfig["buildFiles"].append(directory + "/CMakeLists.txt");

    // libraries
    std::vector<const cmTarget *> targets;
    for (const auto &pair : makefile->GetTargets())
      targets.push_back(&pair.second);
    for (const auto target : makefile->GetImportedTargets())
      targets.push_back(target);
    for (const auto target : targets)
    {
      switch(target->GetType())
      {
        case cmState::EXECUTABLE:
        case cmState::STATIC_LIBRARY:
        case cmState::SHARED_LIBRARY:
        case cmState::MODULE_LIBRARY:
        case cmState::OBJECT_LIBRARY:
          {
            std::string library = target->GetName();

            const std::string config =
              makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");

            if (!config.empty())
              library += '-' + config;

            const std::string abi =
              makefile->GetSafeDefinition("CMAKE_ANDROID_ARCH_ABI");

            if (!abi.empty())
              library += '-' + abi;

            NativeBuildConfig["libraries"][library] =
              ExportTarget(globalGenerator, target,
                           localGenerator, toolchain, config, abi);
            {
              const auto extensions =
                ExportExtensions(globalGenerator, "C", target,
                                 localGenerator, makefile);
              cFileExtensions.insert(extensions.begin(), extensions.end());
            }
            {
              const auto extensions =
                ExportExtensions(globalGenerator, "CXX", target,
                                 localGenerator, makefile);
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

std::set<std::string> cmAndroidGradleBuild
::ExportExtensions(const cmGlobalCommonGenerator *globalGenerator,
                   const std::string language,
                   const cmTarget *target,
                   const cmLocalGenerator *localGenerator,
                   const cmMakefile *makefile)
{
  if(target->IsImported())
    return std::set<std::string>();

  std::set<std::string> extensions;

  const std::string config = makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");

  std::vector<cmSourceFile *> sources;
  cmGeneratorTarget *gt =
    localGenerator->FindGeneratorTargetToUse(target->GetName());
  gt->GetSourceFiles(sources, config);

  for (const auto &source : sources)
    if (language == source->GetLanguage())
      extensions.insert(source->GetExtension());

  return extensions;
}

Json::Value cmAndroidGradleBuild
::ExportTarget(const cmGlobalCommonGenerator *globalGenerator,
               const cmTarget *target,
               const cmLocalGenerator *localGenerator,
               const std::string &toolchain,
               const std::string &config,
               const std::string &abi)
{
  Json::Value NativeLibrary;

  // buildCommand
  if (!target->IsImported())
  {
    const cmMakefile *makefile = localGenerator->GetMakefile();
    const std::string cmakeCommand = cmSystemTools::ConvertToOutputPath(
      makefile->GetRequiredDefinition("CMAKE_COMMAND"));
    const std::string homeOutputDirectory = cmSystemTools::ConvertToOutputPath(
      makefile->GetHomeOutputDirectory());
    std::string buildCommand = cmakeCommand +
      " --build " + homeOutputDirectory + " --target " + target->GetName();
    NativeLibrary["buildCommand"] = buildCommand;
  }

  // buildType
  if (!config.empty())
    NativeLibrary["buildType"] = cmSystemTools::LowerCase(config);

  // artifactName
  NativeLibrary["artifactName"] = target->GetName();

  // abi
  if (!abi.empty())
    NativeLibrary["abi"] = abi;

  // output
  cmGeneratorTarget *gt =
    localGenerator->FindGeneratorTargetToUse(target->GetName());
  if (target->GetType() != cmState::OBJECT_LIBRARY)
  {
    const std::string output = gt->GetLocation(config);
    if (!output.empty() && !cmSystemTools::IsNOTFOUND(output.c_str()))
      NativeLibrary["output"] = output;
  }

  // toolchain
  NativeLibrary["toolchain"] = toolchain;

  // files
  if (!target->IsImported())
  {
    std::vector<cmSourceFile *> sources;
    gt->GetSourceFiles(sources, config);
    for (const auto &source : sources)
    {
      const std::string language = source->GetLanguage();
      if (language == "C" || language == "CXX")
        NativeLibrary["files"].append(ExportSource(globalGenerator, target,
                                                   localGenerator, source));
    }
  }

  return NativeLibrary;
}

Json::Value cmAndroidGradleBuild
::ExportSource(const cmGlobalCommonGenerator *globalGenerator,
               const cmTarget *target,
               const cmLocalGenerator *generator,
               const cmSourceFile *source)
{
  Json::Value NativeSourceFile;

  // src
  NativeSourceFile["src"] = source->GetFullPath();

  // workingDirectory
  std::string workingDirectory;
  if (globalGenerator->GetName() == "Ninja")
    workingDirectory = generator->GetMakefile()->GetHomeOutputDirectory();
  else
    workingDirectory = generator->GetMakefile()->GetCurrentBinaryDirectory();
  workingDirectory =
    generator->Convert(workingDirectory, cmLocalGenerator::FULL);
  NativeSourceFile["workingDirectory"] = workingDirectory;

  // flagsString
  cmGeneratorTarget *gt =
    generator->FindGeneratorTargetToUse(target->GetName());
  cmAndroidGradleTargetGenerator tg(gt);
  NativeSourceFile["flags"] = tg.ExportFlags(source);

  return NativeSourceFile;
}

void cmAndroidGradleBuild::cmAndroidGradleTargetGenerator
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
std::string cmAndroidGradleBuild::cmAndroidGradleTargetGenerator
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
  vars.CMTarget = this->GeneratorTarget;
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
