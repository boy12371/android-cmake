// TODO: remove this file when upstreaming.

#ifndef cmExtraAndroidGradleGenerator_h
#define cmExtraAndroidGradleGenerator_h

#include "cmExternalMakefileProjectGenerator.h"

/** \brief This does nothing anymore.
 *
 * Left here for compatibility with Android Studio.
 *
 * TODO: remove this when no longer necessary.
 */

class cmExtraAndroidGradleGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraAndroidGradleGenerator()
    : cmExternalMakefileProjectGenerator()
  {
    SupportedGlobalGenerators.push_back("Unix Makefiles");
    SupportedGlobalGenerators.push_back("Ninja");
  }

  virtual std::string GetName() const override
  {
    return cmExtraAndroidGradleGenerator::GetActualName();
  }

  static std::string GetActualName() { return "Android Gradle"; }

  static cmExternalMakefileProjectGenerator* New()
  {
    return new cmExtraAndroidGradleGenerator;
  }

  virtual void GetDocumentation(cmDocumentationEntry& entry,
                                const std::string& fullName) const override
  {
    entry.Name = GetName();
    entry.Brief = "Generates Android Gradle build files.";
  }

  virtual void Generate() override {}
};

#endif
