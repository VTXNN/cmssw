import FWCore.ParameterSet.Config as cms
from L1Trigger.TrackTrigger.TrackQualityParams_cfi import *
from L1Trigger.TrackTrigger.KFTrackQualityParams_cfi import *

TrackFindingTrackletProducerKF_params = cms.PSet (

  InputTag             = cms.InputTag( "TrackletTracksFromTrackletEmulation", "Level1TTTracks"), #
  LabelKFin            = cms.string  ( "TrackFindingTrackletProducerKFin"  ),                    #
  LabelKF              = cms.string  ( "TrackFindingTrackletProducerKF"    ),                    #
  LabelTT              = cms.string  ( "TrackFindingTrackletProducerTT"    ),                    #
  LabelAS              = cms.string  ( "TrackFindingTrackletProducerAS"    ),                    #
  LabelKFout           = cms.string  ( "TrackFindingTrackletProducerKFout" ),                    #
  BranchAcceptedStubs  = cms.string  ( "StubAccepted"  ),                                        #
  BranchAcceptedTracks = cms.string  ( "TrackAccepted" ),                                        #
  BranchLostStubs      = cms.string  ( "StubLost"      ),                                        #
  BranchLostTracks     = cms.string  ( "TrackLost"     ),                                        #
  CheckHistory         = cms.bool    ( False ),                                                  # checks if input sample production is configured as current process
  EnableTruncation     = cms.bool    ( True  ) ,
  TrackQuality         = cms.bool    ( True ),
  TrackQualityPSet     = cms.PSet    ( TrackQualityParams ),
  KFTrackQualityPSet     = cms.PSet    ( KFTrackQualityParams )
                                                  # enable emulation of truncation, lost stubs are filled in BranchLost

)