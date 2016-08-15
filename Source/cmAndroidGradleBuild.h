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

/** \brief Write Android Gradle build files.
 *
 * This is used to write the JSON file for external native build system
 * integration with the Android Gradle plugin.
 *
 * TODO: add link to the JSON specification after it's published.
 */

class cmAndroidGradleBuild
{
public:
  /**
   * Export the project structure into a JSON file (android_gradle_build.json)
   * at the root of the build directory.
   *
   * This includes:
   * buildFiles        - A list of build files (CMakeLists.txt).
   * cFileExtensions   - A list of file extensions for C.
   * cleanCommands     - A list of commands (actually just one) to clean the
   *                     build directory.
   * cppFileExtensions - A list of file extensions for C++.
   * libraries         - A list of targets (called libraries but also includes
   *                     executables).
   * toolchains        - A list of toolchains (actually just one).
   */
  static void ExportProject(cmGlobalCommonGenerator* globalGenerator);

private:
  /** Export a list of file extensions for the given language.  */
  static std::set<std::string> ExportExtensions(
    const std::string& language, const cmTarget* target,
    const cmLocalGenerator* localGenerator, const cmMakefile* makefile);

  /**
   * Export every target (static libraries, shared libraries, executables) in
   * the project as a JSON object.
   *
   * This includes:
   * abi          - The ABI.
   * artifactName - The target name.
   * buildCommand - The build command to build the target.
   * buildType    - The build type (Debug or Release).
   * files        - The list of source files for the target
   *                (including those of dependant object libraries).
   * output       - The output file location of the target.
   * toolchain    - The ID of the toolchain used to build the target.
   */
  static Json::Value ExportTarget(cmGlobalCommonGenerator* globalGenerator,
                                  const cmTarget* target,
                                  const cmLocalGenerator* localGenerator,
                                  const std::string& toolchain,
                                  const std::string& config,
                                  const std::string& abi);

  /**
   * Export every source file in a given target as a JSON object.
   *
   * This includes:
   * flags            - The flags used when compiling the source file.
   * src              - The location of the source file.
   * workingDirectory - The working directory of the compiler when compiling the
   *                    source file.
   */
  static Json::Value ExportSource(
    const cmGlobalCommonGenerator* globalGenerator,
    const cmLocalGenerator* localGenerator, const cmSourceFile* source,
    cmGeneratorTarget* generatorTarget);

  /** Dummy target generator used to export flags.
   *
   * We need a target generator to get to all of the information we need to
   * reconstruct the flags.
   */

  class cmAndroidGradleTargetGenerator : public cmCommonTargetGenerator
  {
  public:
    cmAndroidGradleTargetGenerator(cmGeneratorTarget* gt)
      // TODO: remove cmOutputConverter::NONE when upstreaming.
      // the parameter was later removed.
      : cmCommonTargetGenerator(cmOutputConverter::NONE, gt)
      , LocalGenerator(
          // This looks suspicious and possibly illegal,
          // but it's what cmCommonTargetGenerator does.
          static_cast<cmLocalAndroidGradleGenerator*>(gt->GetLocalGenerator()))
    {
    }

    std::string ExportFlags(const cmSourceFile* source);

  private:
    void AddIncludeFlags(std::string& flags,
                         const std::string& language) override;

    /** Dummy local generator used to expand rule variables.
     *
     * We need a local generator to expand rule variables, but it needs to be
     * a friend of the target generator.
     */

    class cmLocalAndroidGradleGenerator : public cmLocalCommonGenerator
    {
      friend class cmAndroidGradleTargetGenerator;
    } * LocalGenerator;
  };
};

#endif
