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
 * This does nothing anymore, left here for compatibility with Android Studio.
 * TODO: remove this when no longer necessary.
 */
class cmExtraAndroidGradleGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraAndroidGradleGenerator() : cmExternalMakefileProjectGenerator()
  {
    SupportedGlobalGenerators.push_back("Unix Makefiles");
    SupportedGlobalGenerators.push_back("Ninja");
  }

  virtual std::string GetName() const override
  { return cmExtraAndroidGradleGenerator::GetActualName(); }

  static std::string GetActualName()
  { return "Android Gradle"; }

  static cmExternalMakefileProjectGenerator* New()
  { return new cmExtraAndroidGradleGenerator; }

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const override
  {
    entry.Name = GetName();
    entry.Brief = "Generates Android Gradle build files.";
  }

  virtual void Generate() override {}

};

#endif
