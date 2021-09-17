/*
Track Quality Body file

C.Brown & C.Savard 07/2020
*/

#include "L1Trigger/TrackTrigger/interface/KFTrackQuality.h"

//Constructors

KFTrackQuality::KFTrackQuality() {}

KFTrackQuality::KFTrackQuality(edm::ParameterSet& qualityParams) {
  // Unpacks EDM parameter set itself to save unecessary processing within TrackProducers

  setONNXModel(edm::FileInPath(qualityParams.getParameter<std::string>("ONNXmodel")),
                 qualityParams.getParameter<std::string>("ONNXInputName"));
  
}

float KFTrackQuality::setKFTrackQuality(std::vector<float> Features) {
    // Setup ONNX input and output names and arrays
    std::vector<std::string> ortinput_names;
    std::vector<std::string> ortoutput_names;

    cms::Ort::FloatArrays ortinput;
    cms::Ort::FloatArrays ortoutputs;

    cms::Ort::ONNXRuntime Runtime(this->ONNXmodel_.fullPath());  //Setup ONNX runtime

    ortinput_names.push_back(this->ONNXInputName_);
    ortoutput_names = Runtime.getOutputNames();

    //ONNX runtime recieves a vector of vectors of floats so push back the input
    // vector of float to create a 1,1,21 ortinput
    ortinput.push_back(Features);

    // batch_size 1 as only one set of transformed features is being processed
    int batch_size = 1;
    // Run classification
    ortoutputs = Runtime.run(ortinput_names, ortinput, {}, ortoutput_names, batch_size);
    
    return ortoutputs[1][1];
  }



void KFTrackQuality::setONNXModel(edm::FileInPath const& ONNXmodel,
                                std::string const& ONNXInputName) {
  ONNXmodel_ = ONNXmodel;
  ONNXInputName_ = ONNXInputName;
}
