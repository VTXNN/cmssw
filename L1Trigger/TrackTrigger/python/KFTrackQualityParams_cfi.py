import FWCore.ParameterSet.Config as cms

KFTrackQualityParams = cms.PSet( ONNXmodel = cms.string("L1Trigger/TrackTrigger/data/NewKFKF.onnx"),
                                 ONNXInputName = cms.string("feature_input")
                              )
