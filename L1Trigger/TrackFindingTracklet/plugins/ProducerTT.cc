#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"

#include "L1Trigger/TrackTrigger/interface/Setup.h"
#include "L1Trigger/TrackerTFP/interface/DataFormats.h"
#include "L1Trigger/TrackTrigger/interface/TrackQuality.h"
#include "L1Trigger/TrackTrigger/interface/KFTrackQuality.h"

#include <string>
#include <numeric>

using namespace std;
using namespace edm;
using namespace trackerTFP;
using namespace tt;

namespace trackFindingTracklet {

  /*! \class  trackFindingTracklet::ProducerTT
   *  \brief  Converts KF output into TTTracks
   *  \author Thomas Schuh
   *  \date   2021, Aug
   */
  class ProducerTT : public stream::EDProducer<> {
  public:
    explicit ProducerTT(const ParameterSet&);
    ~ProducerTT() override {}

  private:
    void beginRun(const Run&, const EventSetup&) override;
    void produce(Event&, const EventSetup&) override;
    void endJob() {}

    // ED input token of kf stubs
    EDGetTokenT<StreamsStub> edGetTokenStubs_;
    // ED input token of kf tracks
    EDGetTokenT<StreamsTrack> edGetTokenTracks_;
    // ED output token for TTTracks
    EDPutTokenT<TTTracks> edPutToken_;
    // Setup token
    ESGetToken<Setup, SetupRcd> esGetTokenSetup_;
    // DataFormats token
    ESGetToken<DataFormats, DataFormatsRcd> esGetTokenDataFormats_;
    // configuration
    ParameterSet iConfig_;
    // helper class to store configurations
    const Setup* setup_;
    // helper class to extract structured data from TTDTC::Frames
    const DataFormats* dataFormats_;
    //Track Quality BDT on Full TTTrack
    bool trackQuality_;
    edm::ParameterSet trackQualityParams;
    std::unique_ptr<TrackQuality> trackQualityModel;

    edm::ParameterSet KFtrackQualityParams;
    std::unique_ptr<KFTrackQuality> KFtrackQualityModel;
  };

  ProducerTT::ProducerTT(const ParameterSet& iConfig) :
    iConfig_(iConfig)
  {
    const string& label = iConfig.getParameter<string>("LabelKF");
    const string& branchStubs = iConfig.getParameter<string>("BranchAcceptedStubs");
    const string& branchTracks = iConfig.getParameter<string>("BranchAcceptedTracks");
    // book in- and output ED products
    edGetTokenStubs_ = consumes<StreamsStub>(InputTag(label, branchStubs));
    edGetTokenTracks_ = consumes<StreamsTrack>(InputTag(label, branchTracks));
    edPutToken_ = produces<TTTracks>(branchTracks);
    // book ES products
    esGetTokenSetup_ = esConsumes<Setup, SetupRcd, Transition::BeginRun>();
    esGetTokenDataFormats_ = esConsumes<DataFormats, DataFormatsRcd, Transition::BeginRun>();
    // initial ES products
    setup_ = nullptr;
    dataFormats_ = nullptr;
    trackQuality_ = iConfig.getParameter<bool>("TrackQuality");
    if (trackQuality_) {
      trackQualityParams = iConfig.getParameter<edm::ParameterSet>("TrackQualityPSet");
      trackQualityModel = std::make_unique<TrackQuality>(trackQualityParams);
      KFtrackQualityParams = iConfig.getParameter<edm::ParameterSet>("KFTrackQualityPSet");
      KFtrackQualityModel = std::make_unique<KFTrackQuality>(KFtrackQualityParams);
    }
  }

  void ProducerTT::beginRun(const Run& iRun, const EventSetup& iSetup) {
    // helper class to store configurations
    setup_ = &iSetup.getData(esGetTokenSetup_);
    if (!setup_->configurationSupported())
      return;
    // check process history if desired
    if (iConfig_.getParameter<bool>("CheckHistory"))
      setup_->checkHistory(iRun.processHistory());
    // helper class to extract structured data from TTDTC::Frames
    dataFormats_ = &iSetup.getData(esGetTokenDataFormats_);
  }

  void ProducerTT::produce(Event& iEvent, const EventSetup& iSetup) {
    // empty KFout product
    TTTracks ttTracks;
    // read in KF Product and produce KFout product
    if (setup_->configurationSupported()) {
      Handle<StreamsStub> handleStubs;
      iEvent.getByToken<StreamsStub>(edGetTokenStubs_, handleStubs);
      const StreamsStub& streamsStubs = *handleStubs.product();
      Handle<StreamsTrack> handleTracks;
      iEvent.getByToken<StreamsTrack>(edGetTokenTracks_, handleTracks);
      const StreamsTrack& streamsTracks = *handleTracks.product();
      // count number of kf tracks
      int nTracks(0);
      for (const StreamTrack& stream : streamsTracks)
        nTracks += accumulate(stream.begin(), stream.end(), 0, [](int& sum, const FrameTrack& frame){ return sum += frame.first.isNonnull() ? 1 : 0; });
      ttTracks.reserve(nTracks);
      // convert kf track frames per channel and stub frames per channel and layer to TTTracks
      for (int channel = 0; channel < (int)streamsTracks.size(); channel++) {
        const int offset = channel * setup_->numLayers();
        int iTrk(0);
        for (const FrameTrack& frameTrack : streamsTracks[channel]) {
          if (frameTrack.first.isNull())
            continue;
          // convert stub frames to kf stubs
          vector<StubKF> stubs;
          stubs.reserve(setup_->numLayers());
          for (int layer = 0; layer < setup_->numLayers(); layer++) {
            const FrameStub& frameStub = streamsStubs[offset + layer][iTrk];
            if (frameStub.first.isNonnull())
              stubs.emplace_back(frameStub, dataFormats_, layer);
          }

          // convert track frame to kf track
          TrackKF track(frameTrack, dataFormats_);
          vector<float> KFBDT(28);
          vector<double> KFvector(31);
          KFBDT[0] = floor((2*track.inv2R())/5.20424e-07);
          KFBDT[1] = floor((2*track.cot())/0.000244141);
          KFBDT[2] = floor((2*track.zT())/0.00999469);
          KFBDT[3] = floor((2*track.phiT())/0.000340885);

          KFvector[0] = track.inv2R();
          KFvector[1] = track.cot();
          KFvector[2] = track.zT();
          KFvector[3] = track.phiT();
          KFvector[4] = track.match();
          KFvector[5] = track.sectorEta();
          KFvector[6] = track.sectorPhi();

          int stubnum = 0;
          for (const StubKF& stub : stubs) {
            KFBDT[3+6*stubnum+1] = floor(stub.r()/0.0399788);
            KFBDT[3+6*stubnum+2] = floor(stub.phi()/4.26106e-05);
            KFBDT[3+6*stubnum+3] = floor(stub.z()/0.0399788);
            KFBDT[3+6*stubnum+4] = floor(stub.dPhi()/4.26106e-05);
            KFBDT[3+6*stubnum+5] = floor(stub.dZ()/0.0399788);
            KFBDT[3+6*stubnum+6] = stub.layer();

            KFvector[6+6*stubnum+1] = stub.r();
            KFvector[6+6*stubnum+2] = stub.phi();
            KFvector[6+6*stubnum+3] = stub.z();
            KFvector[6+6*stubnum+4] = stub.dPhi();
            KFvector[6+6*stubnum+5] = stub.dZ();
            KFvector[6+6*stubnum+6] = stub.layer();
            stubnum++;
          }

          // convert kf track and kf stubs to TTTrack
          TTTrack<Ref_Phase2TrackerDigi_> FullTrack = track.ttTrack(stubs);
          float MVA2 = -999;
          if (trackQuality_) {
            trackQualityModel->setTrackQuality(FullTrack);
            MVA2 = KFtrackQualityModel->setKFTrackQuality(KFBDT);
          }
          FullTrack.settrkMVA2(MVA2);
          FullTrack.setKFTrack(KFvector);
          ttTracks.emplace_back(FullTrack);
          iTrk++;
        }
      }
    }
    // store products
    iEvent.emplace(edPutToken_, move(ttTracks));
  }

} // namespace trackFindingTracklet

DEFINE_FWK_MODULE(trackFindingTracklet::ProducerTT);