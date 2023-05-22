#include <napi.h>
#include <string>

#include <cups/cups.h>
#include <cups/ppd.h>

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

  Napi::Value PrintDirect(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env();

    if (info.Length() < 5 ) {
       Napi::TypeError::New(env, "Expected 5 arguments").ThrowAsJavaScriptException();
    }

    std::string data = info[0].As<Napi::String>().Utf8Value();
    std::string printername = info[1].As<Napi::String>().Utf8Value();
    std::string docname = info[2].As<Napi::String>().Utf8Value();
    std::string type = info[3].As<Napi::String>().Utf8Value(); //CUPS_FORMAT_RAW

    int job_id = cupsCreateJob(CUPS_HTTP_DEFAULT, printername.c_str(), docname.c_str(), 0, NULL);

    if(job_id == 0) {
        Napi::TypeError::New(env, cupsLastErrorString()).ThrowAsJavaScriptException();
    }

    if(HTTP_CONTINUE != cupsStartDocument(CUPS_HTTP_DEFAULT, printername.c_str(), job_id, docname.c_str(), CUPS_FORMAT_RAW, 1 /*last document*/)) {
        Napi::TypeError::New(env, cupsLastErrorString()).ThrowAsJavaScriptException();
    }

    /* cupsWriteRequestData can be called as many times as needed */
    //TODO: to split big buffer
    if (HTTP_CONTINUE != cupsWriteRequestData(CUPS_HTTP_DEFAULT, data.c_str(), data.size())) {
        cupsFinishDocument(CUPS_HTTP_DEFAULT, printername.c_str());
        Napi::TypeError::New(env, cupsLastErrorString()).ThrowAsJavaScriptException();
    }

    cupsFinishDocument(CUPS_HTTP_DEFAULT, printername.c_str());
      
    return Napi::Number::New(env, job_id);
  }

  Napi::Value GetPrinters(const Napi::CallbackInfo& info) {

      Napi::Env env = info.Env();

      cups_dest_t *printers = NULL;
      int printers_size = cupsGetDests(&printers);

      Napi::Array array = Napi::Array::New(env, printers_size);

      cups_dest_t *printer = printers;

      Napi::Object obj1;

      for(int i = 0; i < printers_size; ++i, ++printer){

        obj1 = Napi::Object::New(env);
        obj1.Set("name", printer -> name);
        obj1.Set("isDefault", static_cast<bool>(printer->is_default));
        array[i] = obj1;
      }      

      return array;
  }
};

// The macro announces that instances of the class `ExampleAddon` will be
// created for each instance of the add-on that must be loaded into Node.js.
NODE_API_ADDON(ExampleAddon)