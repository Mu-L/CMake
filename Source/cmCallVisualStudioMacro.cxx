/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCallVisualStudioMacro.h"

#include <sstream>

#include <cmext/string_view>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if defined(_MSC_VER)
#  define HAVE_COMDEF_H
#endif

// Just for this file:
//
static bool LogErrorsAsMessages;

#if defined(HAVE_COMDEF_H)

#  include <comdef.h>

// Copied from a correct comdef.h to avoid problems with deficient versions
// of comdef.h that exist in the wild... Fixes issue #7533.
//
#  ifdef _NATIVE_WCHAR_T_DEFINED
#    ifdef _DEBUG
#      pragma comment(lib, "comsuppwd.lib")
#    else
#      pragma comment(lib, "comsuppw.lib")
#    endif
#  else
#    ifdef _DEBUG
#      pragma comment(lib, "comsuppd.lib")
#    else
#      pragma comment(lib, "comsupp.lib")
#    endif
#  endif

//! Use ReportHRESULT to make a cmSystemTools::Message after calling
//! a COM method that may have failed.
#  define ReportHRESULT(hr, context)                                          \
    if (FAILED(hr)) {                                                         \
      if (LogErrorsAsMessages) {                                              \
        std::ostringstream _hresult_oss;                                      \
        _hresult_oss.flags(std::ios::hex);                                    \
        _hresult_oss << context << " failed HRESULT, hr = 0x" << hr << '\n';  \
        _hresult_oss.flags(std::ios::dec);                                    \
        _hresult_oss << __FILE__ << "(" << __LINE__ << ")";                   \
        cmSystemTools::Message(_hresult_oss.str());                           \
      }                                                                       \
    }

//! Using the given instance of Visual Studio, call the named macro
HRESULT InstanceCallMacro(IDispatch* vsIDE, std::string const& macro,
                          std::string const& args)
{
  HRESULT hr = E_POINTER;

  _bstr_t macroName(macro.c_str());
  _bstr_t macroArgs(args.c_str());

  if (0 != vsIDE) {
    DISPID dispid = (DISPID)-1;
    wchar_t execute_command[] = L"ExecuteCommand";
    OLECHAR* name = execute_command;

    hr =
      vsIDE->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
    ReportHRESULT(hr, "GetIDsOfNames(ExecuteCommand)");

    if (SUCCEEDED(hr)) {
      VARIANTARG vargs[2];
      DISPPARAMS params;
      VARIANT result;
      EXCEPINFO excep;
      UINT arg = (UINT)-1;

      // No VariantInit or VariantClear calls are necessary for
      // these two vargs. They are both local _bstr_t variables
      // that remain in scope for the duration of the Invoke call.
      //
      V_VT(&vargs[1]) = VT_BSTR;
      V_BSTR(&vargs[1]) = macroName;
      V_VT(&vargs[0]) = VT_BSTR;
      V_BSTR(&vargs[0]) = macroArgs;

      params.rgvarg = &vargs[0];
      params.rgdispidNamedArgs = 0;
      params.cArgs = sizeof(vargs) / sizeof(vargs[0]);
      params.cNamedArgs = 0;

      VariantInit(&result);

      memset(&excep, 0, sizeof(excep));

      hr = vsIDE->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                         DISPATCH_METHOD, &params, &result, &excep, &arg);

      std::ostringstream oss;
      /* clang-format off */
      oss << "\nInvoke(ExecuteCommand)\n"
             "  Macro: " << macro << "\n"
             "  Args: " << args << '\n';
      /* clang-format on */

      if (DISP_E_EXCEPTION == hr) {
        /* clang-format off */
        oss << "DISP_E_EXCEPTION EXCEPINFO:" << excep.wCode << "\n"
               "  wCode: " << excep.wCode << "\n"
               "  wReserved: " << excep.wReserved << '\n';
        /* clang-format on */
        if (excep.bstrSource) {
          oss << "  bstrSource: " << (char const*)(_bstr_t)excep.bstrSource
              << '\n';
        }
        if (excep.bstrDescription) {
          oss << "  bstrDescription: "
              << (char const*)(_bstr_t)excep.bstrDescription << '\n';
        }
        if (excep.bstrHelpFile) {
          oss << "  bstrHelpFile: " << (char const*)(_bstr_t)excep.bstrHelpFile
              << '\n';
        }
        /* clang-format off */
        oss << "  dwHelpContext: " << excep.dwHelpContext << "\n"
               "  pvReserved: " << excep.pvReserved << "\n"
               "  pfnDeferredFillIn: "
            << reinterpret_cast<void*>(excep.pfnDeferredFillIn) << "\n"
               "  scode: " << excep.scode << '\n';
        /* clang-format on */
      }

      std::string exstr(oss.str());
      ReportHRESULT(hr, exstr.c_str());

      VariantClear(&result);
    }
  }

  return hr;
}

//! Get the Solution object from the IDE object
HRESULT GetSolutionObject(IDispatch* vsIDE, IDispatchPtr& vsSolution)
{
  HRESULT hr = E_POINTER;

  if (0 != vsIDE) {
    DISPID dispid = (DISPID)-1;
    wchar_t solution[] = L"Solution";
    OLECHAR* name = solution;

    hr =
      vsIDE->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
    ReportHRESULT(hr, "GetIDsOfNames(Solution)");

    if (SUCCEEDED(hr)) {
      DISPPARAMS params;
      VARIANT result;
      EXCEPINFO excep;
      UINT arg = (UINT)-1;

      params.rgvarg = 0;
      params.rgdispidNamedArgs = 0;
      params.cArgs = 0;
      params.cNamedArgs = 0;

      VariantInit(&result);

      memset(&excep, 0, sizeof(excep));

      hr = vsIDE->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                         DISPATCH_PROPERTYGET, &params, &result, &excep, &arg);
      ReportHRESULT(hr, "Invoke(Solution)");

      if (SUCCEEDED(hr)) {
        vsSolution = V_DISPATCH(&result);
      }

      VariantClear(&result);
    }
  }

  return hr;
}

//! Get the FullName property from the Solution object
HRESULT GetSolutionFullName(IDispatch* vsSolution, std::string& fullName)
{
  HRESULT hr = E_POINTER;

  if (0 != vsSolution) {
    DISPID dispid = (DISPID)-1;
    wchar_t full_name[] = L"FullName";
    OLECHAR* name = full_name;

    hr = vsSolution->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT,
                                   &dispid);
    ReportHRESULT(hr, "GetIDsOfNames(FullName)");

    if (SUCCEEDED(hr)) {
      DISPPARAMS params;
      VARIANT result;
      EXCEPINFO excep;
      UINT arg = (UINT)-1;

      params.rgvarg = 0;
      params.rgdispidNamedArgs = 0;
      params.cArgs = 0;
      params.cNamedArgs = 0;

      VariantInit(&result);

      memset(&excep, 0, sizeof(excep));

      hr = vsSolution->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                              DISPATCH_PROPERTYGET, &params, &result, &excep,
                              &arg);
      ReportHRESULT(hr, "Invoke(FullName)");

      if (SUCCEEDED(hr)) {
        fullName = (std::string)(_bstr_t)V_BSTR(&result);
      }

      VariantClear(&result);
    }
  }

  return hr;
}

//! Get the FullName property from the Solution object, given the IDE object
HRESULT GetIDESolutionFullName(IDispatch* vsIDE, std::string& fullName)
{
  IDispatchPtr vsSolution;
  HRESULT hr = GetSolutionObject(vsIDE, vsSolution);
  ReportHRESULT(hr, "GetSolutionObject");

  if (SUCCEEDED(hr)) {
    GetSolutionFullName(vsSolution, fullName);
    ReportHRESULT(hr, "GetSolutionFullName");
  }

  return hr;
}

//! Get all running objects from the Windows running object table.
//! Save them in a map by their display names.
HRESULT GetRunningInstances(std::map<std::string, IUnknownPtr>& mrot)
{
  // mrot == Map of the Running Object Table

  IRunningObjectTablePtr runningObjectTable;
  IEnumMonikerPtr monikerEnumerator;
  IMonikerPtr moniker;
  ULONG numFetched = 0;

  HRESULT hr = GetRunningObjectTable(0, &runningObjectTable);
  ReportHRESULT(hr, "GetRunningObjectTable");

  if (SUCCEEDED(hr)) {
    hr = runningObjectTable->EnumRunning(&monikerEnumerator);
    ReportHRESULT(hr, "EnumRunning");
  }

  if (SUCCEEDED(hr)) {
    hr = monikerEnumerator->Reset();
    ReportHRESULT(hr, "Reset");
  }

  if (SUCCEEDED(hr)) {
    while (S_OK == monikerEnumerator->Next(1, &moniker, &numFetched)) {
      std::string runningObjectName;
      IUnknownPtr runningObjectVal;
      IBindCtxPtr ctx;

      hr = CreateBindCtx(0, &ctx);
      ReportHRESULT(hr, "CreateBindCtx");

      if (SUCCEEDED(hr)) {
        LPOLESTR displayName = 0;
        hr = moniker->GetDisplayName(ctx, 0, &displayName);
        ReportHRESULT(hr, "GetDisplayName");
        if (displayName) {
          runningObjectName = (std::string)(_bstr_t)displayName;
          CoTaskMemFree(displayName);
        }

        hr = runningObjectTable->GetObject(moniker, &runningObjectVal);
        ReportHRESULT(hr, "GetObject");
        if (SUCCEEDED(hr)) {
          mrot.insert(std::make_pair(runningObjectName, runningObjectVal));
        }
      }

      numFetched = 0;
      moniker = 0;
    }
  }

  return hr;
}

//! Do the two file names refer to the same Visual Studio solution? Or are
//! we perhaps looking for any and all solutions?
bool FilesSameSolution(std::string const& slnFile, std::string const& slnName)
{
  if (slnFile == "ALL"_s || slnName == "ALL"_s) {
    return true;
  }

  // Otherwise, make lowercase local copies, convert to Unix slashes, and
  // see if the resulting strings are the same:
  std::string s1 = cmSystemTools::LowerCase(slnFile);
  std::string s2 = cmSystemTools::LowerCase(slnName);
  cmSystemTools::ConvertToUnixSlashes(s1);
  cmSystemTools::ConvertToUnixSlashes(s2);

  return s1 == s2;
}

//! Find instances of Visual Studio with the given solution file
//! open. Pass "ALL" for slnFile to gather all running instances
//! of Visual Studio.
HRESULT FindVisualStudioInstances(std::string const& slnFile,
                                  std::vector<IDispatchPtr>& instances)
{
  std::map<std::string, IUnknownPtr> mrot;

  HRESULT hr = GetRunningInstances(mrot);
  ReportHRESULT(hr, "GetRunningInstances");

  if (SUCCEEDED(hr)) {
    std::map<std::string, IUnknownPtr>::iterator it;
    for (it = mrot.begin(); it != mrot.end(); ++it) {
      if (cmHasLiteralPrefix(it->first, "!VisualStudio.DTE.")) {
        IDispatchPtr disp(it->second);
        if (disp != (IDispatch*)0) {
          std::string slnName;
          hr = GetIDESolutionFullName(disp, slnName);
          ReportHRESULT(hr, "GetIDESolutionFullName");

          if (FilesSameSolution(slnFile, slnName)) {
            instances.push_back(disp);

            // std::cout << "Found Visual Studio instance." << std::endl;
            // std::cout << "  ROT entry name: " << it->first << std::endl;
            // std::cout << "  ROT entry object: "
            //          << (IUnknown*) it->second << std::endl;
            // std::cout << "  slnFile: " << slnFile << std::endl;
            // std::cout << "  slnName: " << slnName << std::endl;
          }
        }
      }
    }
  }

  return hr;
}

#endif // defined(HAVE_COMDEF_H)

int cmCallVisualStudioMacro::GetNumberOfRunningVisualStudioInstances(
  std::string const& slnFile)
{
  int count = 0;

  LogErrorsAsMessages = false;

#if defined(HAVE_COMDEF_H)
  HRESULT hr = CoInitialize(0);
  ReportHRESULT(hr, "CoInitialize");

  if (SUCCEEDED(hr)) {
    std::vector<IDispatchPtr> instances;
    hr = FindVisualStudioInstances(slnFile, instances);
    ReportHRESULT(hr, "FindVisualStudioInstances");

    if (SUCCEEDED(hr)) {
      count = static_cast<int>(instances.size());
    }

    // Force release all COM pointers before CoUninitialize:
    instances.clear();

    CoUninitialize();
  }
#else
  (void)slnFile;
#endif

  return count;
}

//! Get all running objects from the Windows running object table.
//! Save them in a map by their display names.
int cmCallVisualStudioMacro::CallMacro(std::string const& slnFile,
                                       std::string const& macro,
                                       std::string const& args,
                                       bool const logErrorsAsMessages)
{
  int err = 1; // no comdef.h

  LogErrorsAsMessages = logErrorsAsMessages;

#if defined(HAVE_COMDEF_H)
  err = 2; // error initializing

  HRESULT hr = CoInitialize(0);
  ReportHRESULT(hr, "CoInitialize");

  if (SUCCEEDED(hr)) {
    std::vector<IDispatchPtr> instances;
    hr = FindVisualStudioInstances(slnFile, instances);
    ReportHRESULT(hr, "FindVisualStudioInstances");

    if (SUCCEEDED(hr)) {
      err = 0; // no error

      std::vector<IDispatchPtr>::iterator it;
      for (it = instances.begin(); it != instances.end(); ++it) {
        hr = InstanceCallMacro(*it, macro, args);
        ReportHRESULT(hr, "InstanceCallMacro");

        if (FAILED(hr)) {
          err = 3; // error attempting to call the macro
        }
      }

      if (instances.empty()) {
        // no instances to call

        // cmSystemTools::Message(
        //  "cmCallVisualStudioMacro::CallMacro no instances found to call",
        //  "Warning");
      }
    }

    // Force release all COM pointers before CoUninitialize:
    instances.clear();

    CoUninitialize();
  }
#else
  (void)slnFile;
  (void)macro;
  (void)args;
  if (LogErrorsAsMessages) {
    cmSystemTools::Message("cmCallVisualStudioMacro::CallMacro is not "
                           "supported on this platform");
  }
#endif

  if (err && LogErrorsAsMessages) {
    std::ostringstream oss;
    oss << "cmCallVisualStudioMacro::CallMacro failed, err = " << err;
    cmSystemTools::Message(oss.str());
  }

  return 0;
}
