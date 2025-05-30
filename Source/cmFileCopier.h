/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_map>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmFileTimeCache.h"

class cmExecutionStatus;
class cmMakefile;

// File installation helper class.
struct cmFileCopier
{
  cmFileCopier(cmExecutionStatus& status, char const* name = "COPY");
  virtual ~cmFileCopier();

  bool Run(std::vector<std::string> const& args);

protected:
  cmExecutionStatus& Status;
  cmMakefile* Makefile;
  char const* Name;
  bool Always = false;
  cmFileTimeCache FileTimes;
  std::unordered_map<std::string, bool> DirEmptyCache;

  // Whether to install a file not matching any expression.
  bool MatchlessFiles = true;

  // Permissions for files and directories installed by this object.
  mode_t FilePermissions = 0;
  mode_t DirPermissions = 0;

  // Properties set by pattern and regex match rules.
  struct MatchProperties
  {
    bool Exclude = false;
    mode_t Permissions = 0;
  };
  struct MatchRule
  {
    cmsys::RegularExpression Regex;
    MatchProperties Properties;
    std::string RegexString;
    MatchRule(std::string const& regex)
      : Regex(regex)
      , RegexString(regex)
    {
    }
  };
  std::vector<MatchRule> MatchRules;

  // Get the properties from rules matching this input file.
  MatchProperties CollectMatchProperties(std::string const& file);

  bool SetPermissions(std::string const& toFile, mode_t permissions);

  // Translate an argument to a permissions bit.
  bool CheckPermissions(std::string const& arg, mode_t& permissions);

  bool InstallSymlinkChain(std::string& fromFile, std::string& toFile);
  bool InstallSymlink(std::string const& fromFile, std::string const& toFile);
  virtual bool InstallFile(std::string const& fromFile,
                           std::string const& toFile,
                           MatchProperties match_properties);
  bool InstallDirectory(std::string const& source,
                        std::string const& destination,
                        MatchProperties match_properties);
  virtual bool Install(std::string const& fromFile, std::string const& toFile);
  virtual std::string const& ToName(std::string const& fromName);

  enum Type
  {
    TypeFile,
    TypeDir,
    TypeLink
  };
  virtual void ReportCopy(std::string const&, Type, bool) {}
  virtual bool ReportMissing(std::string const& fromFile);

  MatchRule* CurrentMatchRule = nullptr;
  bool UseGivenPermissionsFile = false;
  bool UseGivenPermissionsDir = false;
  bool UseSourcePermissions = true;
  bool ExcludeEmptyDirectories = false;
  bool FollowSymlinkChain = false;
  std::string Destination;
  std::string FilesFromDir;
  std::vector<std::string> Files;

  enum
  {
    DoingNone,
    DoingError,
    DoingDestination,
    DoingFilesFromDir,
    DoingFiles,
    DoingPattern,
    DoingRegex,
    DoingPermissionsFile,
    DoingPermissionsDir,
    DoingPermissionsMatch,
    DoingLast1
  };
  int Doing = DoingNone;

  virtual bool Parse(std::vector<std::string> const& args);
  virtual bool CheckKeyword(std::string const& arg);
  virtual bool CheckValue(std::string const& arg);

  void NotBeforeMatch(std::string const& arg);
  void NotAfterMatch(std::string const& arg);
  virtual void DefaultFilePermissions();
  virtual void DefaultDirectoryPermissions();

  bool GetDefaultDirectoryPermissions(mode_t** mode);
};
