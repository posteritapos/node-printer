const fs = require('fs');
const path = require("path");

const oldPath =  path.resolve(__dirname,'lib','node_printer.node');
const newPath = path.resolve(__dirname,'lib',`node_printer_${process.platform}_${process.arch}.node`);

fs.rename(oldPath, newPath, (err) => {
  if (err) {
    console.error('Error renaming file:', err);
  } else {
    console.log('File renamed successfully.');
  }
});