/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2016 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAndroidGradleBuild_h
#define cmAndroidGradleBuild_h

#include "cmCommonTargetGenerator.h"
#include "cmGlobalCommonGenerator.h"
#include "cmLocalCommonGenerator.h"
#include "cm_jsoncpp_value.h"

class cmGeneratedFileStream;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmTarget;

/** \class cmAndroidGradleBuild
 * \brief Write Android Gradle build files
 */
class cmAndroidGradleBuild
{
public:

  static void ExportProject(const cmGlobalCommonGenerator *globalGenerator);

private:

  static std::set<std::string>
    ExportExtensions(const cmGlobalCommonGenerator *globalGenerator,
                     const std::string language,
                     const cmTarget *target,
                     const cmLocalGenerator *localGenerator,
                     const cmMakefile *makefile);

  static Json::Value
    ExportTarget(const cmGlobalCommonGenerator *globalGenerator,
                 const cmTarget *target,
                 const cmLocalGenerator *localGenerator,
                 const std::string &toolchain,
                 const std::string &config,
                 const std::string &abi);

  static Json::Value
    ExportSource(const cmGlobalCommonGenerator *globalGenerator,
                 const cmTarget *target,
                 const cmLocalGenerator *localGenerator,
                 const cmSourceFile *source);

  /** Dummy target generator for ExportFlags.  */
  class cmAndroidGradleTargetGenerator : public cmCommonTargetGenerator {
  public:
    cmAndroidGradleTargetGenerator(cmGeneratorTarget *gt)
      : cmCommonTargetGenerator{cmOutputConverter::NONE, gt},
        LocalGenerator{
          static_cast<cmLocalAndroidGradleGenerator *>(gt->GetLocalGenerator())
        }
    {}
    std::string ExportFlags(const cmSourceFile *source);
  private:
    void AddIncludeFlags(std::string &flags,
                         const std::string &language) override;

    /** Dummy local generator for ExpandRuleVariables.  */
    class cmLocalAndroidGradleGenerator : public cmLocalCommonGenerator
    {
      friend class cmAndroidGradleTargetGenerator;
    } *LocalGenerator;
  };
};

#endif
