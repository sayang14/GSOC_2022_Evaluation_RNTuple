R__LOAD_LIBRARY(ROOTNTuple)
#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
 
#include <TCanvas.h>
#include <TH1I.h>
#include <TROOT.h>
#include <TString.h>
 
#include <cstring>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <utility>

using RNTupleModel = ROOT::Experimental::RNTupleModel;
using RNTupleReader = ROOT::Experimental::RNTupleReader;
using RNTupleWriter = ROOT::Experimental::RNTupleWriter;

constexpr char const* kNTupleFileName = "gsoc_eval_RNTuple.root";

std::vector<vector<string>> h;
std::pair<std::shared_ptr<int>,std::shared_ptr<float>> p;
std::map<int,std::shared_ptr<int>> mp1;
std::map<int,std::shared_ptr<float>> mp2;


void Ingest() {
    char filename[100];
    //I specifically mentioned the tuitorials directory since I found most of the datasets are stored there
    std::cout << "Enter the csv filename path(no more than 100 characters) present in the installation/tutorials directory of ROOT):"<<std::endl;
    std::cin >> filename;
    ifstream fin(gROOT->GetTutorialDir() + "/tree/" + filename);
    assert(fin.is_open());
    //creating unique pointer to an empty data model
    auto model = RNTupleModel::Create();
    
    std::string record_header,word;
    getline(fin,record_header);
    std::cout<<record_header<<std::endl;
    std::istringstream iss(record_header);
    int i = 0;
    while(getline(iss,word,','))
    {
      if(word.find("int")!= string::npos){
        mp1[i] = model->MakeField<int>(word);
        mp2[i] = nullptr;
        i++;
        }
        else if(word.find("float")!= string::npos) {
        mp2[i] = model->MakeField<float>(word);
        mp1[i] = nullptr;
        i++;
        } else continue;
    }   
    
    // We hand-over the data model to a newly created ntuple of name "new_ntuple", stored in kNTupleFileName
    auto ntuple = RNTupleWriter::Recreate(std::move(model), "new_ntuple", kNTupleFileName);
    
    std::string record;
    //std::cout<<v1.size()<<" "<<v2.size()<<std::endl;
    while(std::getline(fin,record)) {
       std::istringstream iss(record);
       //iss>>*fld1>>*fld2>>*fld3>>*fld4;
       for(int j=0;j<i;j++)
       {
          if(mp1[j]==nullptr)iss>>*mp2[j];
          else iss>>*mp1[j];
          if(iss.peek()==',')iss.ignore();
       }
       ntuple->Fill();
    }  
}

void Analyze() {
   // Get a unique pointer to empty RNTuple models
   auto model = RNTupleModel::Create();
   
   std::string entry;
   std::cout<<"Enter the entry whose distribution you wish to see in the following way(Name:Type)(Avoid any spaces)!"<<std::endl;
   std::cin>>entry;
   
   //defining field(fld from Ingest function where it was declared) that is needed for reading
   if(entry.find("int")!=string::npos){auto fld = model->MakeField<int>(entry);
     p.first = fld;
   }
   else if(entry.find("float")!=string::npos){auto fld = model->MakeField<float>(entry);
     p.second = fld;
   }	
   
   // Quick overview of the ntuple and list of fields.
   auto ntuple = RNTupleReader::Open(std::move(model),"new_ntuple", kNTupleFileName);
   ntuple->PrintInfo();
   
   std::cout << "The first entry in JSON format:" << std::endl;
   ntuple->Show(0);
   
   auto c = new TCanvas("c", "", 200, 10, 700, 500);
   TH1I h("h", " Distribution for your entry", 100, -100, 100);
   h.SetFillColor(40);
   
   
   for (auto entryId : *ntuple) {
      ntuple->LoadEntry(entryId);
      if(entry.find("int")!=string::npos)h.Fill(*p.first);
      else if(entry.find("float")!=string::npos)h.Fill(*p.second); 
   }
   
   h.DrawCopy();
}

void gsoc_eval_RNTuple() {
   Ingest();
   Analyze();
}








