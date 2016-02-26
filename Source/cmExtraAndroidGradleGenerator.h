/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2016 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExtraAndroidGradleGenerator_h
#define cmExtraAndroidGradleGenerator_h

#include "cmExternalMakefileProjectGenerator.h"
#include "cmCommonTargetGenerator.h"
#include "cmLocalCommonGenerator.h"

#include "cm_jsoncpp_value.h"

class cmGeneratedFileStream;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
class cmTarget;

/** \class cmExtraAndroidGradleGenerator
 * \brief Write Android Gradle build files
 */
class cmExtraAndroidGradleGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraAndroidGradleGenerator();

  virtual std::string GetName() const override
  { return cmExtraAndroidGradleGenerator::GetActualName(); }

  static std::string GetActualName()
  { return "Android Gradle"; }

  static cmExternalMakefileProjectGenerator* New()
  { return new cmExtraAndroidGradleGenerator; }

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const override;

  virtual void Generate() override;

private:

  using cmLocalGenerators = std::vector<cmLocalGenerator*>;

  void GenerateProject(const cmLocalGenerators &generators);

  std::set<std::string> ExportExtensions(const std::string language,
                                         const cmTarget *target,
                                         const cmMakefile *makefile);

  Json::Value ExportTarget(const cmTarget *target,
                           const cmLocalGenerator *generator,
                           const std::string &toolchain,
                           const std::string &config,
                           const std::string &abi);

  Json::Value ExportSource(const cmTarget *target,
                           const cmLocalGenerator *generator,
                           const cmSourceFile *source);

  /** Dummy target generator for flags.  */
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

    /** For ExpandRuleVariables.  */
    class cmLocalAndroidGradleGenerator : public cmLocalCommonGenerator
    {
      friend class cmAndroidGradleTargetGenerator;
    } *LocalGenerator;
  };

};

#endif
