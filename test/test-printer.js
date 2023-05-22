var x = require('../printer');

console.log(x.getPrinters());


x.printDirect({data:"print from Node.JS buffer\n\n\n\n" // or simple String: "some text"
    , printer:'EPSON_TM-T20' // printer name, if missing then will print to default printer
    , type: 'RAW' // type: RAW, TEXT, PDF, JPEG, .. depends on platform
    , success:function(jobID){
        console.log("sent to printer with ID: "+jobID);
    }
    , error:function(err){console.log(err);}
});