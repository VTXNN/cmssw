/*
Track Quality Header file

C.Brown 28/07/20
*/

#ifndef L1Trigger_TrackTrigger_interface_KFTrackQuality_h
#define L1Trigger_TrackTrigger_interface_KFTrackQuality_h

#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include <string>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "PhysicsTools/ONNXRuntime/interface/ONNXRuntime.h"

#include "DataFormats/L1TrackTrigger/interface/TTTrack.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"

class KFTrackQuality {
public:
  //Default Constructor
  KFTrackQuality();

  KFTrackQuality(edm::ParameterSet& qualityParams);

  //Default Destructor
  ~KFTrackQuality() = default;


  // Passed by reference a track without MVA filled, method fills the track's MVA field
  float setKFTrackQuality(std::vector<float> Features);


  void setONNXModel(edm::FileInPath const& ONNXmodel,
                    std::string const& ONNXInputName);

private:
  // Private Member Data
  edm::FileInPath ONNXmodel_;
  std::string ONNXInputName_;

};
#endif
