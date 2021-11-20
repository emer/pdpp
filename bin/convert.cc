// convert an  old network specific pdp.pat input-target file to a
// new file with pattern per line;

#include <stream.h>
#include <strstream.h>
#include <fstream.h>
#include <std.h>
#include <stdio.h>
#include <String.h>

main(){
  String infile,outfile,nfile;
  String tempstring,patname;
    int ninputs,noutputs,i;
  
  
  cout << "This file takes a xxx.pat file and converts it to \n" <<
    "the new format by using the xxx.str file's ninputs\n" <<
      " and nouputs fields\n\n";
  cout << "Input xxx.pat File:";
  cin >> infile;
  if (infile.contains(".pat")) {
    nfile = infile.before(".pat") + ".net";
  }
  cout << "\nOutput File:";
  cin >> outfile;
  cout << "\n";
  
  ifstream  netfile(nfile);
  
  while(netfile >> tempstring) {
    if (tempstring == "ninputs") {
      netfile >> ninputs;
      cout << "ninputs = " << ninputs << "\n";
    }
    if (tempstring == "noutputs") {
      netfile >> noutputs;
      cout << "noutputs = " << noutputs << "\n";
    }
  }
  netfile.close();

  ofstream  outputfile(outfile);
  ifstream  inputfile(infile);
  
  while(inputfile >> patname) {
    outputfile << patname << "i ";
    for(i=0;i<ninputs;i++){
      inputfile >> tempstring;
      outputfile << tempstring + " ";
    }
    outputfile << "\n";
    outputfile << patname << "o ";
    for(i=0;i<noutputs;i++){
      inputfile >> tempstring;
      outputfile << tempstring + " ";
    }
    outputfile << "\n";
  }
  
  inputfile.close();
  outputfile.close();
  
}
