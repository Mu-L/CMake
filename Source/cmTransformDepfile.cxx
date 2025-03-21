/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmTransformDepfile.h"

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmsys/FStream.hxx"

#include "cmGccDepfileReader.h"
#include "cmGccDepfileReaderTypes.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
void WriteFilenameGcc(cmsys::ofstream& fout, std::string const& filename)
{
  for (auto c : filename) {
    switch (c) {
      case ' ':
        fout << "\\ ";
        break;
      case '\\':
        fout << "\\\\";
        break;
      default:
        fout << c;
        break;
    }
  }
}

void WriteDepfile(cmDepfileFormat format, cmsys::ofstream& fout,
                  cmLocalGenerator const& lg,
                  cmGccDepfileContent const& content)
{
  std::function<std::string(std::string const&)> formatPath =
    [&lg](std::string const& path) -> std::string {
    return lg.MaybeRelativeToTopBinDir(path);
  };
  if (lg.GetGlobalGenerator()->GetName() == "Xcode") {
    // full paths must be preserved for Xcode compliance
    formatPath = [](std::string const& path) -> std::string { return path; };
  }

  for (auto const& dep : content) {
    bool first = true;
    for (auto const& rule : dep.rules) {
      if (!first) {
        fout << " \\\n  ";
      }
      first = false;
      WriteFilenameGcc(fout, formatPath(rule));
    }
    fout << ':';
    for (auto const& path : dep.paths) {
      fout << " \\\n  ";
      WriteFilenameGcc(fout, formatPath(path));
    }
    fout << '\n';
  }

  if (format == cmDepfileFormat::MakeDepfile) {
    // In this case, phony targets must be added for all dependencies
    fout << "\n";
    for (auto const& dep : content) {
      for (auto const& path : dep.paths) {
        fout << "\n";
        WriteFilenameGcc(fout, formatPath(path));
        fout << ":\n";
      }
    }
  }
}

void WriteMSBuildAdditionalInputs(cmsys::ofstream& fout,
                                  cmLocalGenerator const& lg,
                                  cmGccDepfileContent const& content)
{
  if (content.empty()) {
    return;
  }

  // Write a UTF-8 BOM so MSBuild knows the encoding when reading the file.
  static char const utf8bom[] = { static_cast<char>(0xEF),
                                  static_cast<char>(0xBB),
                                  static_cast<char>(0xBF) };
  fout.write(utf8bom, sizeof(utf8bom));

  // Write the format expected by MSBuild CustomBuild AdditionalInputs.
  char const* sep = "";
  for (auto const& c : content) {
    for (std::string path : c.paths) {
      if (!cmSystemTools::FileIsFullPath(path)) {
        path = cmSystemTools::CollapseFullPath(path,
                                               lg.GetCurrentBinaryDirectory());
      }
      std::replace(path.begin(), path.end(), '/', '\\');
      fout << sep << path;
      sep = ";";
    }
  }
  fout << "\n";
}
}

bool cmTransformDepfile(cmDepfileFormat format, cmLocalGenerator const& lg,
                        std::string const& infile, std::string const& outfile)
{
  cmGccDepfileContent content;
  if (cmSystemTools::FileExists(infile)) {
    auto result =
      cmReadGccDepfile(infile.c_str(), lg.GetCurrentBinaryDirectory());
    if (!result) {
      return false;
    }
    content = *std::move(result);
  } else {
    lg.GetMakefile()->IssueMessage(
      MessageType::WARNING,
      cmStrCat("Expected depfile does not exist.\n  ", infile));
  }

  cmSystemTools::MakeDirectory(cmSystemTools::GetFilenamePath(outfile));
  cmsys::ofstream fout(outfile.c_str());
  if (!fout) {
    return false;
  }
  switch (format) {
    case cmDepfileFormat::GccDepfile:
    case cmDepfileFormat::MakeDepfile:
      WriteDepfile(format, fout, lg, content);
      break;
    case cmDepfileFormat::MSBuildAdditionalInputs:
      WriteMSBuildAdditionalInputs(fout, lg, content);
      break;
  }
  return true;
}
