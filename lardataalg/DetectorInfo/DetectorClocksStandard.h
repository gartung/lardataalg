////////////////////////////////////////////////////////////////////////
//
// DetectorClocks.h
//
//     This class provides electronics various electronics clocks. Currently supports
//     three types of clocks: TPC, Optical, and Trigger in order to support the
//     MicroBooNE experiment.
//
//     Formally known as TimeService.
//
////////////////////////////////////////////////////////////////////////

#ifndef LARDATAALG_DETECTORINFO_DETECTORCLOCKSSTANDARD_H
#define LARDATAALG_DETECTORINFO_DETECTORCLOCKSSTANDARD_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "lardataalg/DetectorInfo/ElecClock.h"
#include "lardataalg/DetectorInfo/DetectorClocks.h"

namespace fhicl { class ParameterSet; }

namespace detinfo{

  /**
   * @brief Implementation of `detinfo::DetectorClocks` interface with fixed
   *        settings from configuration.
   *
   * In this implementation, all quantities are defined at configuration time
   * and are unchanged for the duration of the entire job, with the exception
   * of the trigger time (see below).
   *
   * Configuration parameters
   * =========================
   *
   * All configuration parameters are mandatory, unless explicitly stated.
   *
   * * *G4RefTime* (_nanoseconds_):
   *     @ref DetectorClocksSimulationTime "simulation (Geant4) start time" in
   *     @ref DetectorClocksElectronicsTime "electronics time scale", i.e. when
   *     time `0.0` of simulation happens in the electronics time scale
   * * *TriggerOffsetTPC*: time elapsed between the
   *     @ref DetectorClocksTPCelectronicsStartTime "start of the TPC readout clock"
   *     and the @ref DetectorClocksHardwareTrigger "hardware trigger"; it can
   *     be expressed in one of two ways:
   *       * negative number [&micro;s]: the offset of the start of the TPC
   *         readout clock start respect to the trigger time (where negative
   *         means that the clock starts _before_ the trigger arrives)
   *       * positive number [ticks]: the number of TPC readout clock tick at
   *         which the trigger arrives; despite this being a tick number, it can
   *         be fractional for added precision
   *
   *     For example, `TriggerOffsetTPC` of `-1600.0` means that the TDC clock
   *     starts 1.6 milliseconds before the hardware trigger. `TriggerOffsetTPC`
   *     of `3200.0` means that the trigger arrives at the exact start of tick
   *     3200 of the TPC readout. In this example, if the sampling frequency of
   *     that readout is 2 MHz, these two settings are equivalent.
   * * *FramePeriod* (_microseconds_): duration of an electronics clock frame;
   *     @ref DetectorClocksIntroClocks "all clocks" share the same frame period
   * * *ClockSpeedTPC* (_megahertz_): frequency of the
   *     @ref DetectorClocksIntroClocks "TPC electronics clock"
   * * *ClockSpeedOptical* (_megahertz_): frequency of the
   *     @ref DetectorClocksIntroClocks "optical electronics clock"
   * * *ClockSpeedTrigger* (_megahertz_): frequency of the
   *     @ref DetectorClocksIntroClocks "trigger electronics clock"
   * * *ClockSpeedExternal* (_megahertz_): frequency of the
   *     @ref DetectorClocksIntroClocks "external electronics clock"
   * * *DefaultTrigTime* (_microseconds_): the default
   *     @ref DetectorClocksHardwareTrigger "hardware trigger time", measured in
   *     the @ref DetectorClocksElectronicsTime "electronics time frame"
   * * *DefaultBeamTime* (_microseconds_): the default
   *     @ref DetectorClocksBeamGateOpening "beam gate opening time", measured
   *     in the @ref DetectorClocksElectronicsTime "electronics time frame"
   * * *TrigModuleName* (_string_): input tag for the trigger data product
   *     (see "Trigger time" section below)
   * * *InheritClockConfig* (_boolean_): whether to inherit the configuration
   *     from previous jobs (see "Consistency check" below)
   *
   *
   * Consistency check
   * ------------------
   *
   * The consistency check feature verifies that the current configuration of
   * `detinfo::DetectorClocksStandard` is compatible with the one from previous
   * jobs. It is expected that when this feature is enabled
   * (`InheritClockConfig` is configured to be `true`), either the configuration
   * is overridden to follow the previous ones, or an exception is thrown in
   * case of inconsistency.
   *
   * The service provider (`detinfo::DetectorClocksStandard`) does not provide
   * a facility to inherit configuration from a previous job, but it stores the
   * expectation whether this should happen (`InheritClockConfig()`).
   * The service provider manager is in charge of implementing this feature
   * (see `detinfo::DetectorClocksStandardService` for the manager in the _art_
   * environment).
   *
   *
   * Timing specifics
   * =================
   *
   * For the general timing requirements, see the documentation of
   * `detinfo::DetectorClocks`.
   *
   *
   * Trigger time
   * -------------
   *
   * In this implementation, the trigger time and the beam gate time both
   * default to 0.0 &micro;s.
   *
   * The manager of this provider is expected to set those values by calling
   * `detinfo::DetectorClocksStandard::SetTriggerTime()`. See
   * `detinfo::DetectorClocksStandardService` for the manager in the _art_
   * environment.
   *
   *
   * @bug `ExternalClock()` clock is never initialized!
   *
   */
  class DetectorClocksStandard : public DetectorClocks {

  public:
    DetectorClocksStandard();
    DetectorClocksStandard(fhicl::ParameterSet const& pset);
    DetectorClocksStandard(DetectorClocksStandard const&) = delete;
    virtual ~DetectorClocksStandard() {};

    bool Configure(fhicl::ParameterSet const& pset);
    bool Update(uint64_t ts=0);

    /**
     * @see `detinfo::DetectorClocks::TriggerOffsetTPC()`
     *
     * This offset is set via configuration parameter `TriggerOffsetTPC`.
     */
    virtual double TriggerOffsetTPC() const override
    {
      if (fTriggerOffsetTPC<0)
	return fTriggerOffsetTPC;
      else
	return -fTriggerOffsetTPC/fTPCClock.Frequency(); //convert ticks to us
    }

    /// Returns the @ref DetectorClocksTPCelectronicsStartTime "TPC electronics start time" in @ref DetectorClocksElectronicsTime "electronics time".
    virtual double TPCTime() const override { return doTPCTime(); }

    void debugReport() const;

    /**
     * @brief Returns the input tag of the trigger data product.
     * @return the input tag of the trigger data product (as string)
     *
     * The trigger module name is set directly in the configuration as
     * `TrigModuleName`.
     */
    std::string TrigModuleName() const { return fTrigModuleName; }

    /**
     * @brief Returns the input tag of the trigger data product for G4Ref correctons.
     * @return the input tag of the trigger data product (as string)
     *
     * The trigger module name is set directly in the configuration as
     * `G4RefCorrTrigModuleName`.
     */
    std::string G4RefCorrTrigModuleName() const { return fG4RefCorrTrigModuleName; }

    /// Sets trigger and beam gate time from default configuration values.
    void SetDefaultTriggerTime();

    /// Given Geant4 time [ns], returns relative time [us] w.r.t. electronics time T0
    virtual double G4ToElecTime(double g4_time) const override {return g4_time * 1.e-3 - fG4RefTime; }

    /// Trigger electronics clock time in [us]
    virtual double TriggerTime() const override { return fTriggerTime; }

    /// Beam gate electronics clock time in [us]
    virtual double BeamGateTime() const override { return fBeamGateTime; }

    virtual std::vector<std::string> ConfigNames() const override { return fConfigName; }
    virtual std::vector<double> ConfigValues() const override { return fConfigValue; }

    void SetConfigValue(size_t i, double val) { fConfigValue[i] = val; }

    /**
     * @brief Setter for trigger times.
     * @param trig_time @ref DetectorClocksHardwareTrigger "hardware trigger time" in @ref DetectorClocksElectronicsTime "electronics time scale"
     * @param beam_time @ref DetectorClocksBeamGateOpening "beam gate opening time" in @ref DetectorClocksElectronicsTime "electronics time scale"
     *
     * The @ref DetectorClocksHardwareTrigger "hardware trigger" and
     * @ref DetectorClocksBeamGateOpening "beam gate opening" times are set, and
     * the electronic clocks are updated to store the new trigger time.
     */
    virtual void SetTriggerTime(double trig_time, double beam_time)
    {
      fTriggerTime  = trig_time;
      fBeamGateTime = beam_time;
      fTPCClock.SetTime(trig_time);
      fOpticalClock.SetTime(trig_time);
      fTriggerClock.SetTime(trig_time);
    }

    /**
     * @brief Setter for correction the G4RefTime.
     * @param sim_trig_time @ref DetectorClocksHardwareTrigger "hardware trigger time" in @ref DetectorClocksElectronicsTime "electronics time scale" from simulation
     *
     * The G4RefTime is corrected to use the existing TriggerTime() as a base, rather than
     * the fcl parameter. This is done in particularly to accomodate matching in overlay (data+sim) samples.
     */
    virtual void RebaseG4RefTime(double sim_trig_time)
    {
      fG4RefTime = fG4RefTimeDefault - TriggerTime() + sim_trig_time;
    }

    //
    // Getters of TPC ElecClock
    //
    /// Borrow a const TPC clock with time set to Trigger time [us]
    virtual const ::detinfo::ElecClock& TPCClock() const override
    { return fTPCClock; }

    /// Create a TPC clock for a given time [us] from clock counting start
    virtual ::detinfo::ElecClock TPCClock(double time) const override
	{ return ::detinfo::ElecClock(time,fTPCClock.FramePeriod(),fTPCClock.Frequency());}

    /// Create a TPC clock for a given sample/frame number in TPC clock frequency
    detinfo::ElecClock TPCClock(unsigned int sample,
		       unsigned int frame) const override
      { detinfo::ElecClock clock = TPCClock(); clock.SetTime(sample,frame); return clock; }

    //
    // Getters of Optical ElecClock
    //
    /// Borrow a const Optical clock with time set to Trigger time [us]
    virtual const detinfo::ElecClock& OpticalClock() const override
    { return fOpticalClock; }

    /// Create a Optical clock for a given time [us] from clock counting start
    virtual detinfo::ElecClock OpticalClock(double time) const override
      { return detinfo::ElecClock(time,fOpticalClock.FramePeriod(),fOpticalClock.Frequency());}

    /// Create a Optical clock for a given sample/frame number in Optical clock frequency
    virtual detinfo::ElecClock OpticalClock(unsigned int sample,
			   unsigned int frame) const override
    { detinfo::ElecClock clock = OpticalClock(); clock.SetTime(sample,frame); return clock; }

    //
    // Getters of Trigger ElecClock
    //
    /// Borrow a const Trigger clock with time set to Trigger time [us]
    virtual const detinfo::ElecClock& TriggerClock() const override
    { return fTriggerClock; }

    /// Create a Trigger clock for a given time [us] from clock counting start
    virtual detinfo::ElecClock TriggerClock(double time) const  override
    { return detinfo::ElecClock(time,fTriggerClock.FramePeriod(),fTriggerClock.Frequency());}

    /// Create a Trigger clock for a given sample/frame number in Trigger clock frequency
    virtual detinfo::ElecClock TriggerClock(unsigned int sample,
			   unsigned int frame) const override
    { detinfo::ElecClock clock = TriggerClock(); clock.SetTime(sample,frame); return clock; }

    //
    // Getters of External ElecClock
    //
    /// Borrow a const Trigger clock with time set to External Time [us]
    virtual const detinfo::ElecClock& ExternalClock() const override
    { return fExternalClock; }

    /// Create a External clock for a given time [us] from clock counting start
    /// @bug The frequency is taken from `TriggerClock()`, not `ExternalClock()`
    virtual detinfo::ElecClock ExternalClock(double time) const override
      { return detinfo::ElecClock(time,fExternalClock.FramePeriod(),fTriggerClock.Frequency());}

    /// Create a External clock for a given sample/frame number in External clock frequency
    virtual detinfo::ElecClock ExternalClock(unsigned int sample,
				  unsigned int frame) const override
      { detinfo::ElecClock clock = ExternalClock(); clock.SetTime(sample,frame); return clock; }

    //
    // Getters for time [us] w.r.t. trigger given information from waveform
    //

    /// Given TPC time-tick (waveform index), returns time [us] w.r.t. trigger time stamp
    virtual double TPCTick2TrigTime(double tick) const override
    { return fTPCClock.TickPeriod() * tick + TriggerOffsetTPC(); }
    /// Given TPC time-tick (waveform index), returns time [us] w.r.t. beam gate time
    virtual double TPCTick2BeamTime(double tick) const override
    { return TPCTick2TrigTime(tick) + TriggerTime() - BeamGateTime(); }
    /// Given Optical time-tick (waveform index), sample and frame number, returns time [us] w.r.t. trigger time stamp
    virtual double OpticalTick2TrigTime(double tick, size_t sample, size_t frame) const override
    { return fOpticalClock.TickPeriod() * tick + fOpticalClock.Time(sample,frame) - TriggerTime();  }
    /// Given Optical time-tick (waveform index), sample and frame number, returns time [us] w.r.t. beam gate time stamp
    virtual double OpticalTick2BeamTime(double tick, size_t sample, size_t frame) const override
    { return fOpticalClock.TickPeriod() * tick + fOpticalClock.Time(sample,frame) - BeamGateTime(); }
    /// Given External time-tick (waveform index), sample and frame number, returns time [us] w.r.t. trigger time stamp
    virtual double ExternalTick2TrigTime(double tick, size_t sample, size_t frame) const override
    { return fExternalClock.TickPeriod() * tick + fExternalClock.Time(sample,frame) - TriggerTime(); }
    /// Given External time-tick (waveform index), sample and frame number, returns time [us] w.r.t. beam gate time stamp
    virtual double ExternalTick2BeamTime(double tick, size_t sample, size_t frame) const override
    { return fExternalClock.TickPeriod() * tick + fExternalClock.Time(sample,frame) - BeamGateTime(); }

    /// Returns the specified electronics time in TDC electronics ticks.
    virtual double Time2Tick(double time) const override
      { return doTime2Tick(time); }

    //
    // Getters for time [tdc] (electronics clock counting ... in double precision)
    //

    /// Given TPC time-tick (waveform index), returns electronics clock count [tdc]
    virtual double TPCTick2TDC(double tick) const override
    { return ( doTPCTime() / fTPCClock.TickPeriod() + tick ); }
    /// Given G4 time [ns], returns corresponding TPC electronics clock count [tdc]
    virtual double TPCG4Time2TDC(double g4time) const override
    { return G4ToElecTime(g4time) / fTPCClock.TickPeriod(); }
    /// Given Optical time-tick (waveform index), sample and frame number, returns time electronics clock count [tdc]
    virtual double OpticalTick2TDC(double tick, size_t sample, size_t frame) const override
    { return fOpticalClock.Ticks(sample,frame) + tick; }
    /// Given G4 time [ns], returns corresponding Optical electronics clock count [tdc]
    virtual double OpticalG4Time2TDC(double g4time) const override
    { return G4ToElecTime(g4time) / fOpticalClock.TickPeriod(); }
    /// Given External time-tick (waveform index), sample and frame number, returns time electronics clock count [tdc]
    virtual double ExternalTick2TDC(double tick, size_t sample, size_t frame) const override
    { return fExternalClock.Ticks(sample,frame) + tick; }
    /// Given G4 time [ns], returns corresponding External electronics clock count [tdc]
    virtual double ExternalG4Time2TDC(double g4time) const override
    { return G4ToElecTime(g4time) / fExternalClock.TickPeriod(); }

    //
    // Getters for time [us] (electronics clock counting ... in double precision)
    //
    /// Given TPC time-tick (waveform index), returns electronics clock [us]
    virtual double TPCTick2Time(double tick) const override
    { return doTPCTime() + tick * fTPCClock.TickPeriod(); }
    /// Given Optical time-tick (waveform index), sample and frame number, returns electronics clock [us]
    virtual double OpticalTick2Time(double tick, size_t sample, size_t frame) const override
    { return fOpticalClock.Time(sample,frame) + tick * fOpticalClock.TickPeriod(); }
    /// Given External time-tick (waveform index), sample and frame number, returns electronics clock [us]
    virtual double ExternalTick2Time(double tick, size_t sample, size_t frame) const override
    { return fExternalClock.Time(sample,frame) + tick * fExternalClock.TickPeriod(); }

    //
    // Getters for time [ticks] (waveform index number)
    //

    /// Given electronics clock count [tdc] returns TPC time-tick
    virtual double TPCTDC2Tick(double tdc) const override
    { return ( tdc - doTPCTime() / fTPCClock.TickPeriod() ); }
    /// Given G4 time returns electronics clock count [tdc]
    virtual double TPCG4Time2Tick(double g4time) const override
    { return (G4ToElecTime(g4time) - doTPCTime()) / fTPCClock.TickPeriod(); }

    bool InheritClockConfig() { return fInheritClockConfig; }

    /// Internal function to apply loaded parameters to member attributes
    void ApplyParams();

    /// Internal function used to search for the right configuration set in the data file
    bool IsRightConfig(const fhicl::ParameterSet& ps) const;

  protected:

    std::vector<std::string> fConfigName;

    std::vector<double>      fConfigValue;

    bool fInheritClockConfig;

    std::string fTrigModuleName;
    std::string fG4RefCorrTrigModuleName;

    /// Electronics clock counting start time in G4 time frame [us]
    double fG4RefTime;

    /// A default G4Reftime, typically coming from a FCL file.
    /// Used to allow per event corrections to be applied.
    double fG4RefTimeDefault;

    /// Frame period
    double fFramePeriod;

    /// TPC clock
    ::detinfo::ElecClock fTPCClock;

    /// Optical clock
    ::detinfo::ElecClock fOpticalClock;

    /// Trigger clock
    ::detinfo::ElecClock fTriggerClock;

    /// External clock
    ::detinfo::ElecClock fExternalClock;

    /// Time offset from trigger to TPC readout start
    double fTriggerOffsetTPC;

    /// Trigger time in [us]
    double fTriggerTime;

    /// BeamGate time in [us]
    double fBeamGateTime;

    /// Implementation of `TPCTime()`.
    double doTPCTime() const { return TriggerTime() + TriggerOffsetTPC(); }

    /// Implementation of `Time2Tick()`.
    double doTime2Tick(double time) const
      { return (time - doTPCTime()) / fTPCClock.TickPeriod();  }

  }; // class DetectorClocksStandard

} //namespace detinfo


#endif // LARDATAALG_DETECTORINFO_DETECTORCLOCKSSTANDARD_H
