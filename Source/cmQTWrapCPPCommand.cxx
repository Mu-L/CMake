/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmQTWrapCPPCommand.h"

#include <utility>

#include <cm/memory>

#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmQTWrapCPPCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Get the moc executable to run in the custom command.
  std::string const& moc_exe = mf.GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // Get the variable holding the list of sources.
  std::string const& sourceList = args[1];
  std::string sourceListValue = mf.GetSafeDefinition(sourceList);

  // Create a rule for all sources listed.
  for (std::string const& arg : cmMakeRange(args).advance(2)) {
    cmSourceFile* curr = mf.GetSource(arg);
    // if we should wrap the class
    if (!(curr && curr->GetPropertyAsBool("WRAP_EXCLUDE"))) {
      // Compute the name of the file to generate.
      std::string srcName =
        cmSystemTools::GetFilenameWithoutLastExtension(arg);
      std::string newName =
        cmStrCat(mf.GetCurrentBinaryDirectory(), "/moc_", srcName, ".cxx");
      cmSourceFile* sf = mf.GetOrCreateSource(newName, true);
      if (curr) {
        sf->SetProperty("ABSTRACT", curr->GetProperty("ABSTRACT"));
      }

      // Compute the name of the header from which to generate the file.
      std::string hname;
      if (cmSystemTools::FileIsFullPath(arg)) {
        hname = arg;
      } else {
        if (curr && curr->GetIsGenerated()) {
          hname = mf.GetCurrentBinaryDirectory();
        } else {
          hname = mf.GetCurrentSourceDirectory();
        }
        hname += "/";
        hname += arg;
      }

      // Append the generated source file to the list.
      if (!sourceListValue.empty()) {
        sourceListValue += ";";
      }
      sourceListValue += newName;

      // Create the custom command to generate the file.
      cmCustomCommandLines commandLines =
        cmMakeSingleCommandLine({ moc_exe, "-o", newName, hname });

      std::vector<std::string> depends;
      depends.push_back(moc_exe);
      depends.push_back(hname);

      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetOutputs(newName);
      cc->SetDepends(depends);
      cc->SetCommandLines(commandLines);
      cc->SetComment("Qt Wrapped File");
      mf.AddCustomCommandToOutput(std::move(cc));
    }
  }

  // Store the final list of source files.
  mf.AddDefinition(sourceList, sourceListValue);
  return true;
}
