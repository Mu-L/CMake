/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmExportFileGenerator.h"
#include "cmStateTypes.h"

namespace Json {
class Value;
}

class cmGeneratorTarget;
class cmPackageInfoArguments;

/** \class cmExportPackageInfoGenerator
 * \brief Generate Common Package Specification package information files
 * exporting targets from a build or install tree.
 *
 * cmExportPackageInfoGenerator is the superclass for
 * cmExportBuildPackageInfoGenerator and cmExportInstallPackageInfoGenerator.
 * It contains common code generation routines for the two kinds of export
 * implementations.
 */
class cmExportPackageInfoGenerator : virtual public cmExportFileGenerator
{
public:
  cmExportPackageInfoGenerator(cmPackageInfoArguments arguments);

  using cmExportFileGenerator::GenerateImportFile;

protected:
  std::string const& GetPackageName() const { return this->PackageName; }

  void WritePackageInfo(Json::Value const& packageInfo,
                        std::ostream& os) const;

  // Methods to implement export file code generation.
  bool GenerateImportFile(std::ostream& os) override;

  bool CheckDefaultTargets() const;

  Json::Value GeneratePackageInfo() const;
  Json::Value* GenerateImportTarget(Json::Value& components,
                                    cmGeneratorTarget const* target,
                                    cmStateEnums::TargetType targetType) const;

  void GeneratePackageRequires(Json::Value& package) const;

  using ImportPropertyMap = std::map<std::string, std::string>;
  bool GenerateInterfaceProperties(Json::Value& component,
                                   cmGeneratorTarget const* target,
                                   ImportPropertyMap const& properties) const;
  Json::Value GenerateInterfaceConfigProperties(
    std::string const& suffix, ImportPropertyMap const& properties) const;

  cm::string_view GetImportPrefixWithSlash() const override;

  std::string GetCxxModuleFile(std::string const& /*name*/) const override
  {
    // TODO
    return {};
  }

  void GenerateCxxModuleConfigInformation(std::string const& /*name*/,
                                          std::ostream& /*os*/) const override
  {
    // TODO
  }

  bool NoteLinkedTarget(cmGeneratorTarget const* target,
                        std::string const& linkedName,
                        cmGeneratorTarget const* linkedTarget) override;

private:
  void GenerateInterfaceLinkProperties(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceCompileFeatures(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceCompileDefines(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    ImportPropertyMap const& properties) const;

  void GenerateInterfaceListProperty(
    bool& result, Json::Value& component, cmGeneratorTarget const* target,
    std::string const& outName, cm::string_view inName,
    ImportPropertyMap const& properties) const;

  void GenerateProperty(bool& result, Json::Value& component,
                        cmGeneratorTarget const* target,
                        std::string const& outName, std::string const& inName,
                        ImportPropertyMap const& properties) const;

  std::string const PackageName;
  std::string const PackageVersion;
  std::string const PackageVersionCompat;
  std::string const PackageVersionSchema;
  std::string const PackageDescription;
  std::string const PackageWebsite;
  std::string const PackageLicense;
  std::string const DefaultLicense;
  std::vector<std::string> DefaultTargets;
  std::vector<std::string> DefaultConfigurations;

  std::map<std::string, std::string> LinkTargets;
  std::map<std::string, std::set<std::string>> Requirements;
};
