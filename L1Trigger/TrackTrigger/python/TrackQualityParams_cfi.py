import FWCore.ParameterSet.Config as cms

TrackQualityParams = cms.PSet(qualityAlgorithm = cms.string("GBDT"), #None, Cut, NN, GBDT
                              ONNXmodel = cms.FileInPath("L1Trigger/TrackTrigger/data/NewKFTrack.onnx"),
                              # The ONNX model should be found at this path, if you want a local version of the model:
                              # git clone https://github.com/cms-data/L1Trigger-TrackTrigger.git L1Trigger/TrackTrigger/data
                              ONNXInputName = cms.string("feature_input"),
                              #Vector of strings of training features, in the order that the model was trained with
                              featureNames = cms.vstring(["bit_TanL", "bit_z0", "bendchi2_bin", 
                                                          "nlaymiss_interior", "chi2rz_bin", "chi2rphi_bin","chi2_bin","bit_phi"]),
                              # Parameters for cut based classifier, optimized for L1 Track MET
                              # (Table 3.7  The Phase-2 Upgrade of the CMS Level-1 Trigger http://cds.cern.ch/record/2714892) 
                              maxZ0 = cms.double ( 15. ) ,    # in cm
                              maxEta = cms.double ( 2.4 ) ,
                              chi2dofMax = cms.double( 40. ),
                              bendchi2Max = cms.double( 2.4 ),
                              minPt = cms.double( 2. ),       # in GeV
                              nStubsmin = cms.int32( 4 ),
                              
                              ONNXInvRScale = cms.double(500)  # Scaling InvR to same order of magnitude as other variables for ML models
                              )
