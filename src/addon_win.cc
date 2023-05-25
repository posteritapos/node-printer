#include <napi.h>
#include <string>

#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#include <Wingdi.h>
#pragma  comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <map>
#include <utility>
#include <sstream>
#include <iostream>


class ExampleAddon : public Napi::Addon<ExampleAddon> {
 public:
  ExampleAddon(Napi::Env env, Napi::Object exports) {
    // In the constructor we declare the functions the add-on makes available
    // to JavaScript.
    DefineAddon(exports, {
      InstanceMethod("getPrinters", &ExampleAddon::GetPrinters),
      InstanceMethod("printDirect", &ExampleAddon::PrintDirect)
    });
  }
 private: 

  std::string getLastErrorCodeAndMessage(){
    
    	std::ostringstream s;
    	DWORD erroCode = GetLastError();
    	s << "code: " << erroCode;
    	DWORD retSize;
    	LPTSTR pTemp = NULL;
    	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                FORMAT_MESSAGE_FROM_SYSTEM|
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL,
                                erroCode,
                                LANG_NEUTRAL,
                                (LPTSTR)&pTemp,
                                0,
                                NULL );
        if (retSize && pTemp != NULL) {
	    //pTemp[strlen(pTemp)-2]='\0'; //remove cr and newline character
	    //TODO: check if it is needed to convert c string to std::string
	    std::string stringMessage(pTemp);
	    s << ", message: " << stringMessage;
	    LocalFree((HLOCAL)pTemp);
	}

    	return s.str();
  }
  
  Napi::Value PrintDirect(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env(); 

    if (info.Length() < 5 ) {
       Napi::TypeError::New(env, "Expected 5 arguments").ThrowAsJavaScriptException();
    }    
     
    std::string data = info[0].As<Napi::String>().Utf8Value<size_t>();
    std::string printername = info[1].As<Napi::String>().Utf8Value<size_t>();
    std::string docname = info[2].As<Napi::String>().Utf8Value<size_t>();
    std::string type = info[3].As<Napi::String>().Utf8Value<size_t>(); //RAW

    LPTSTR szPrinterName = (LPTSTR)(printername.c_str());
    LPVOID  lpData = (LPVOID)(data.c_str()); 
    DWORD dwCount = (DWORD)data.size();

    BOOL bStatus = FALSE;
    HANDLE hPrinter = NULL;
    DOC_INFO_1 DocInfo;
    DWORD dwPrtJob = 0L;
    DWORD dwBytesWritten = 0L;    

    // Open a handle to the printer. 
    bStatus = OpenPrinter (szPrinterName, &hPrinter, NULL);

    if (bStatus) {

      // Fill in the structure with info about this "document."
      DocInfo.pDocName = (LPTSTR)(docname.c_str());
      DocInfo.pOutputFile =  NULL;
      DocInfo.pDatatype = (LPTSTR)(type.c_str());

      dwPrtJob = StartDocPrinter (hPrinter, 1, (LPBYTE)&DocInfo);

      if (dwPrtJob > 0) {
            // Send the data to the printer. 
            bStatus = WritePrinter (hPrinter, lpData, dwCount, &dwBytesWritten);
      }
      else
      {
        ClosePrinter(hPrinter);
        Napi::TypeError::New(env, "StartDocPrinterW error: " + getLastErrorCodeAndMessage()).ThrowAsJavaScriptException();
      }
      
      EndDocPrinter (hPrinter);

      // Close the printer handle. 
      bStatus = ClosePrinter(hPrinter);

    }
    else
    {
      Napi::TypeError::New(env, "Failed to open printer: " + printername + " " + getLastErrorCodeAndMessage()).ThrowAsJavaScriptException();
    }

    if (!bStatus || (dwCount != dwBytesWritten)) {
        bStatus = FALSE;
    } else {
        bStatus = TRUE;
    }

    return Napi::Number::New(env, (int)dwPrtJob);    
  }

  Napi::Value GetPrinters(const Napi::CallbackInfo& info) {

      Napi::Env env = info.Env();   

      PRINTER_INFO_2*    list;
      DWORD cnt = 0;
      DWORD sz = 0;
      DWORD Level=2;

      EnumPrinters( PRINTER_ENUM_LOCAL, NULL, Level, NULL, 0, &sz, &cnt );
   
      if( (list = (PRINTER_INFO_2*) malloc( sz )) == 0 ){
          Napi::TypeError::New(env, "Failed to allocate memory").ThrowAsJavaScriptException();
      }

      if(!EnumPrinters(PRINTER_ENUM_LOCAL, NULL, Level, (LPBYTE)list, sz, &sz, &cnt))
      {
        free( list );
        Napi::TypeError::New(env, "Failed to enummerate printers").ThrowAsJavaScriptException();
      }
      
      uint32_t printers_size = (uint32_t)cnt;

      Napi::Array array = Napi::Array::New(env, printers_size);

      Napi::Object obj1;

      for(uint32_t i = 0; i < printers_size; ++i){

        obj1 = Napi::Object::New(env);
        obj1.Set("name", list[i].pPrinterName);
        //obj1.Set("status", list[i].Status);
        //obj1.Set("isDefault", "false");
        array[i] = obj1;
      }   

      free( list );

      return array;
  }
};

// The macro announces that instances of the class `ExampleAddon` will be
// created for each instance of the add-on that must be loaded into Node.js.
NODE_API_ADDON(ExampleAddon)
