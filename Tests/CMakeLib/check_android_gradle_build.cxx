#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cm_jsoncpp_reader.h"

int failure(const std::string &message)
{
  std::cout << "ERROR: " << message << std::endl;
  return EXIT_FAILURE;
}

int checkStringArray(const Json::Value &project,
                     const std::string &name,
                     std::set<std::string> expectedMembers)
{
  const Json::Value array = project[name];
  if (!array.isArray())
    return failure(name + " is not an array.");
  for (const Json::Value &member : array)
  {
    const auto str = member.asString();
    if (expectedMembers.count(str) == 0)
      return failure("Unrecognized member " + str + " in " + name + ".");
    expectedMembers.erase(str);
  }
  for (const auto &member : expectedMembers)
    return failure(name + " doesn't contain " + member+ ".");
  return EXIT_SUCCESS;
}

int checkFiles(const Json::Value library,
               const std::string &language,
               const std::string &artifact,
               const std::string &buildType,
               const cmCacheManager &cache)
{
  const Json::Value files = library["files"];
  if (!files.isArray())
    return failure("files is not an array.");
  const std::string generator =
    cache.GetInitializedCacheValue("CMAKE_GENERATOR");
  const std::string source_dir =
    cache.GetInitializedCacheValue("AndroidGradleBuild_SOURCE_DIR");
  const std::string binary_dir =
    cache.GetInitializedCacheValue("AndroidGradleBuild_BINARY_DIR");
  std::set<std::string> expectedSources;
  if (artifact == "exe")
  {
    expectedSources.insert(source_dir + "/main." + language);
    expectedSources.insert(source_dir + "/" + artifact + "." + language);
    expectedSources.insert(source_dir + "/object/object." + language);
  }
  else
    expectedSources.insert(source_dir + "/" + artifact + "/" +
                           artifact + "." + language);
  std::set<std::string> expectedFlags = {
    "-DDEFINITION",
    "-I" + cmSystemTools::ConvertToOutputPath((source_dir + "/shared").c_str()),
    "-I" + cmSystemTools::ConvertToOutputPath((source_dir + "/static").c_str()),
    "-I" + cmSystemTools::ConvertToOutputPath((source_dir + "/module").c_str()),
    "-I" + cmSystemTools::ConvertToOutputPath((source_dir + "/object").c_str()),
    "-D" + cmSystemTools::UpperCase(language) + "_FLAGS",
    "-D" + cmSystemTools::UpperCase(language) + "_" +
      cmSystemTools::UpperCase(buildType) + "_FLAGS"
  };
  for (const auto &file : files)
  {
    std::set<std::string> expectedMembers = {
      "flags",
      "src",
      "workingDirectory"
    };
    for (const auto &member : file.getMemberNames())
    {
      if (expectedMembers.count(member) == 0)
        return failure("Unrecognized member " + member + " in files.");
      expectedMembers.erase(member);
    }
    for (const auto &member : expectedMembers)
      return failure("files doesn't contain " + member+ ".");

    const std::string src = file["src"].asString();
    if (expectedSources.count(src) == 0)
      return failure("Unrecognized src " + src + " in files.");
    expectedSources.erase(src);

    const std::string workingDirectory = file["workingDirectory"].asString();
    std::string expectedWorkingDirectory = binary_dir;
    if (generator == "Unix Makefiles" && artifact != "exe")
      expectedWorkingDirectory += "/" + artifact;
    if (workingDirectory != expectedWorkingDirectory)
      return failure("workingDirectory doesn't match " +
                     expectedWorkingDirectory + ".");

    const std::string flags = file["flags"].asString();
    for (const auto &flag : expectedFlags)
      if (flags.find(flag) == std::string::npos)
        return failure("Flag " + flag + " not found in file.");
  }
  for (const auto &src : expectedSources)
    return failure("files doesn't contain " + src + ".");
  return EXIT_SUCCESS;
}

int checkLibrary(const Json::Value &libraries,
                 const std::string &language,
                 const std::string &artifact,
                 const std::string &buildType,
                 const std::string &abi,
                 const std::string &toolchain,
                 const cmCacheManager &cache)
{
  const std::string name = language + "_" + artifact;
  const Json::Value library =
    libraries[name + "-" + buildType + "-" + abi];
  if (!library.isObject())
    return failure("library is not a JSON object.");
  std::set<std::string> expectedMembers = {
    "abi",
    "artifactName",
    "buildCommand",
    "buildType",
    "files",
    "output",
    "toolchain"
  };

  for (const auto &member : library.getMemberNames())
  {
    if (expectedMembers.count(member) == 0)
      return failure("Unrecognized member " + member + " in " +
                     name + " library.");
    expectedMembers.erase(member);
  }
  for (const auto &member : expectedMembers)
    return failure(name + " library doesn't contain " + member+ ".");

  if (library["abi"] != abi)
    return failure(name + " abi doesn't match " + abi + ".");
  if (library["artifactName"] != name)
    return failure(name + " artifactName doesn't match " + name + ".");
  const std::string cmake =
    cache.GetInitializedCacheValue("CMAKE_COMMAND");
  const std::string binary_dir =
    cache.GetInitializedCacheValue("AndroidGradleBuild_BINARY_DIR");
  const std::string buildCommand =
    cmSystemTools::ConvertToOutputPath(cmake.c_str()) +
    " --build " +
    cmSystemTools::ConvertToOutputPath(binary_dir.c_str()) +
    " --target " + name;
  if (library["buildCommand"] != buildCommand)
    return failure(name + " buildCommand doesn't match " + buildCommand+ ".");
  if (library["buildType"] != cmSystemTools::LowerCase(buildType))
    return failure(name + " buildType doesn't match " +
                   cmSystemTools::LowerCase(buildType) + ".");
  if (checkFiles(library, language, artifact, buildType, cache))
    return EXIT_FAILURE;
  std::string output;
  if (artifact == "exe")
    output = binary_dir + "/" + name +
      cache.GetInitializedCacheValue("CMAKE_EXECUTABLE_SUFFIX");
  else if (artifact == "shared")
    output = binary_dir + "/" + artifact + "/" +
      cache.GetInitializedCacheValue("CMAKE_SHARED_LIBRARY_PREFIX") +
      name +
      cache.GetInitializedCacheValue("CMAKE_SHARED_LIBRARY_SUFFIX");
  else if (artifact == "static")
    output = binary_dir + "/" + artifact + "/" +
      cache.GetInitializedCacheValue("CMAKE_STATIC_LIBRARY_PREFIX") +
      name +
      cache.GetInitializedCacheValue("CMAKE_STATIC_LIBRARY_SUFFIX");
  else if (artifact == "module")
    output = binary_dir + "/" + artifact + "/" +
      cache.GetInitializedCacheValue("CMAKE_SHARED_MODULE_PREFIX") +
      name +
      cache.GetInitializedCacheValue("CMAKE_SHARED_MODULE_SUFFIX");
  if (library["output"] != output)
    return failure(name + " output doesn't match " + output + ".");
  if (library["toolchain"] != toolchain)
    return failure(name + " toolchain doesn't match " + toolchain + ".");
  return EXIT_SUCCESS;
}

int checkImportedLibrary(const Json::Value &libraries,
                         const std::string &name,
                         const std::string &buildType,
                         const std::string &abi,
                         const std::string &toolchain)
{
  const Json::Value library = libraries[name + "-" + buildType + "-" + abi];
  if (!library.isObject())
    return failure(name + " library is not a JSON object.");
  std::set<std::string> expectedMembers = {
    "abi",
    "artifactName",
    "buildType",
    "toolchain"
  };
  if (name == "imported")
    expectedMembers.insert("output");

  for (const auto &member : library.getMemberNames())
  {
    if (expectedMembers.count(member) == 0)
      return failure("Unrecognized member " + member + " in " +
                     name + " library.");
    expectedMembers.erase(member);
  }
  for (const auto &member : expectedMembers)
    return failure(name + " library doesn't contain " + member+ ".");

  if (library["abi"] != abi)
    return failure(name + " abi doesn't match " + abi + ".");
  if (library["artifactName"] != name)
    return failure(name + " artifactName doesn't match " + name + ".");
  if (library["buildType"] != cmSystemTools::LowerCase(buildType))
    return failure(name + " buildType doesn't match " +
                   cmSystemTools::LowerCase(buildType) + ".");
  if (name == "imported" &&
      library["output"] != "/fake/location/libimported.so")
    return failure(name + " output doesn't match " +
                   "/fake/location/libimported.so.");
  if (library["toolchain"] != toolchain)
    return failure(name + " toolchain doesn't match " + toolchain + ".");
  return EXIT_SUCCESS;
}

int checkLibraries(const Json::Value &project, const cmCacheManager &cache)
{
  const Json::Value libraries = project["libraries"];
  if (!libraries.isObject())
    return failure("libraries is not a JSON object.");
  const std::string toolchain = project["toolchains"].getMemberNames().front();
  const std::string buildType =
    cache.GetInitializedCacheValue("CMAKE_BUILD_TYPE");
  const std::string abi =
    cache.GetInitializedCacheValue("CMAKE_ANDROID_ARCH_ABI");
  std::set<std::string> expectedLibraries;
  std::vector<std::string> languages = { "c", "cpp" };
  std::vector<std::string> artifacts = {
    "exe", "shared", "static", "module"
  };
  for (const auto &language : languages)
    for (const auto &artifact : artifacts)
      expectedLibraries.insert(
        language + "_" + artifact + "-" + buildType + "-" + abi);
  expectedLibraries.insert("imported-" + buildType + "-" + abi);
  for (const auto &library : libraries.getMemberNames())
  {
    if (expectedLibraries.count(library) == 0)
      return failure("Unrecognized library " + library + " in libraries.");
    expectedLibraries.erase(library);
  }
  for (const auto &library : expectedLibraries)
    return failure("Libraries doesn't contain " + library + ".");
  for (const auto &language : languages)
    for (const auto &artifact : artifacts)
      if (checkLibrary(libraries, language, artifact, buildType, abi, toolchain,
                       cache))
        return EXIT_FAILURE;
  for (const auto &library : { "imported" })
    if (checkImportedLibrary(libraries, library, buildType, abi, toolchain))
      return EXIT_FAILURE;
  return EXIT_SUCCESS;
}

int checkToolchains(const Json::Value &project, const cmCacheManager &cache)
{
  const Json::Value toolchains = project["toolchains"];
  if (!toolchains.isObject())
    return failure("toolchains is not a JSON object.");
  Json::Value::Members members = toolchains.getMemberNames();
  if (members.size() != 1)
    return failure("toolchains should contain exactly one toolchain.");

  const Json::Value toolchain = toolchains[members.front()];
  std::set<std::string> expectedMembers = {
    "cCompilerExecutable",
    "cppCompilerExecutable",
  };
  for (const auto &member : toolchain.getMemberNames())
  {
    if (expectedMembers.count(member) == 0)
      return failure("Unrecognized member" + member + " in toolchain.");
    expectedMembers.erase(member);
  }
  for (const auto &member : expectedMembers)
    return failure("Toolchain doesn't contain " + member+ ".");

  std::string cc = cache.GetInitializedCacheValue("CMAKE_C_COMPILER");
  std::string cxx = cache.GetInitializedCacheValue("CMAKE_CXX_COMPILER");
  if (toolchain["cCompilerExecutable"] != cc)
    return failure("cCompilerExecutable doesn't match CMAKE_C_COMPILER.");
  if (toolchain["cppCompilerExecutable"] != cxx)
    return failure("cppCompilerExecutable doesn't match CMAKE_CXX_COMPILER.");

  return EXIT_SUCCESS;
}

int checkProject(const Json::Value &project, const cmCacheManager &cache)
{
  std::set<std::string> expectedMembers = {
    "buildFiles",
    "cFileExtensions",
    "cppFileExtensions",
    "cleanCommands",
    "libraries",
    "toolchains"
  };
  if (!project.isObject())
    return failure("Project is not a JSON object.");
  for (const auto &member : project.getMemberNames())
  {
    if (expectedMembers.count(member) == 0)
      return failure("Unrecognized member " + member + " in project.");
    expectedMembers.erase(member);
  }
  for (const auto &member : expectedMembers)
    return failure("Project doesn't contain " + member + ".");

  const std::string cmake = cache.GetInitializedCacheValue("CMAKE_COMMAND");
  const std::string source_dir =
    cache.GetInitializedCacheValue("AndroidGradleBuild_SOURCE_DIR");
  const std::string binary_dir =
    cache.GetInitializedCacheValue("AndroidGradleBuild_BINARY_DIR");
  return
    checkStringArray(project, "buildFiles", {
                     source_dir + "/CMakeLists.txt",
                     source_dir + "/shared/CMakeLists.txt",
                     source_dir + "/static/CMakeLists.txt",
                     source_dir + "/module/CMakeLists.txt",
                     source_dir + "/object/CMakeLists.txt",
                     }) ||
    checkStringArray(project, "cFileExtensions", { "c" }) ||
    checkStringArray(project, "cppFileExtensions", { "cpp" }) ||
    checkStringArray(project, "cleanCommands", {
                     cmSystemTools::ConvertToOutputPath(cmake.c_str()) +
                     " --build " +
                     cmSystemTools::ConvertToOutputPath(binary_dir.c_str()) +
                     " --target clean"
                     }) ||
    checkLibraries(project, cache) ||
    checkToolchains(project, cache);
}

int main()
{
  cmCacheManager cache;
  std::set<std::string> excludes;
  std::set<std::string> includes;
  if (!cache.LoadCache(cmSystemTools::GetCurrentWorkingDirectory(), 1,
                       excludes, includes))
    return failure("Failed to load CMake cache.");
  if (strcmp(cache.GetInitializedCacheValue("CMAKE_SYSTEM_NAME"), "Android"))
    return failure("CMAKE_SYSTEM_NAME doesn't match Android.");
  std::ifstream ifs("android_gradle_build.json");
  if (!ifs.good())
    return failure("JSON missing or unreadable.");
  std::stringstream ss;
  ss << ifs.rdbuf();
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(ss.str(), root))
    return failure("Failed to parse JSON.");
  return checkProject(root, cache);
}
