/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

enum cmNinjaTargetDepends
{
  DependOnTargetArtifact,
  DependOnTargetOrdering
};

using cmNinjaDeps = std::vector<std::string>;
using cmNinjaVars = std::map<std::string, std::string>;

class cmNinjaRule
{
public:
  cmNinjaRule(std::string name)
    : Name(std::move(name))
  {
  }

  std::string Name;
  std::string Command;
  std::string Description;
  std::string Comment;
  std::string DepFile;
  std::string DepType;
  std::string RspFile;
  std::string RspContent;
  std::string Restat;
  bool Generator = false;
};

class cmNinjaBuild
{
public:
  cmNinjaBuild() = default;
  cmNinjaBuild(std::string rule)
    : Rule(std::move(rule))
  {
  }

  std::string Comment;
  std::string Rule;
  cmNinjaDeps Outputs;
  cmNinjaDeps ImplicitOuts;
  cmNinjaDeps WorkDirOuts; // For cmake_ninja_workdir.
  cmNinjaDeps ExplicitDeps;
  cmNinjaDeps ImplicitDeps;
  cmNinjaDeps OrderOnlyDeps;
  cmNinjaVars Variables;
  std::string RspFile;
};
