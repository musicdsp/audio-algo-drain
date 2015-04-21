/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test/debug.h>
#include <etk/etk.h>
#include <audio/algo/drain/Equalizer.h>
#include <etk/os/FSNode.h>
#include <etk/chrono.h>

#include <unistd.h>
#undef __class__
#define __class__ "test"


class Performance {
	private:
		std11::chrono::steady_clock::time_point m_timeStart;
		std11::chrono::steady_clock::time_point m_timeStop;
		std11::chrono::nanoseconds m_totalTimeProcessing;
		std11::chrono::nanoseconds m_minProcessing;
		std11::chrono::nanoseconds m_maxProcessing;
		int32_t m_totalIteration;
	public:
		Performance() :
		  m_totalTimeProcessing(0),
		  m_minProcessing(99999999999999LL),
		  m_maxProcessing(0),
		  m_totalIteration(0) {
			
		}
		void tic() {
			m_timeStart = std11::chrono::steady_clock::now();
		}
		void toc() {
			m_timeStop = std11::chrono::steady_clock::now();
			std11::chrono::nanoseconds time = m_timeStop - m_timeStart;
			m_minProcessing = std::min(m_minProcessing, time);
			m_maxProcessing = std::max(m_maxProcessing, time);
			m_totalTimeProcessing += time;
			m_totalIteration++;
			
		}
		
		std11::chrono::nanoseconds getTotalTimeProcessing() {
			return m_totalTimeProcessing;
		}
		std11::chrono::nanoseconds getMinProcessing() {
			return m_minProcessing;
		}
		std11::chrono::nanoseconds getMaxProcessing() {
			return m_maxProcessing;
		}
		int32_t getTotalIteration() {
			return m_totalIteration;
		}
		
};

float performanceEqualizerType(audio::format _type) {
	std::vector<float> input;
	input.resize(1024, 0);
	std::vector<float> output;
	output.resize(input.size()*10, 0);
	double sampleRate = 48000;
	{
		double phase = 0;
		double baseCycle = 2.0*M_PI/sampleRate * 480.0;
		for (int32_t iii=0; iii<input.size(); iii++) {
			input[iii] = cos(phase) * 5.0;
			phase += baseCycle;
			if (phase >= 2*M_PI) {
				phase -= 2*M_PI;
			}
		}
	}
	APPL_INFO("Start Equalizer (1 biquad) ... " << _type);
	Performance perfo;
	audio::algo::drain::Equalizer algo;
	// configure in float
	algo.init(48000, 1, _type);
	// add basic biquad ...
	algo.addBiquad(0.01, 0.02, 0.03, 0.05, 0.06);
	// set 1024 test
	for (int32_t iii=0; iii<1024; ++iii) {
		perfo.tic();
		size_t sizeOut = output.size();
		algo.process(&output[0], &input[0], input.size());
		perfo.toc();
		usleep(1000);
	}
	APPL_INFO("    blockSize=" << input.size() << " sample");
	APPL_INFO("    min < avg < max =" << perfo.getMinProcessing().count() << "ns < "
	                                  << perfo.getTotalTimeProcessing().count()/perfo.getTotalIteration() << "ns < "
	                                  << perfo.getMaxProcessing().count() << "ns ");
	float avg = (float(((perfo.getTotalTimeProcessing().count()/perfo.getTotalIteration())*sampleRate)/double(input.size()))/1000000000.0)*100.0;
	APPL_INFO("    min < avg < max= " << (float((perfo.getMinProcessing().count()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "% < "
	                                  << avg << "% < "
	                                  << (float((perfo.getMaxProcessing().count()*sampleRate)/double(input.size()))/1000000000.0)*100.0 << "%");
	APPL_PRINT("type=" << _type << ": " << avg << "%");
	return avg;
}

void performanceEqualizer() {
	performanceEqualizerType(audio::format_double);
	performanceEqualizerType(audio::format_float);
	performanceEqualizerType(audio::format_int8);
	performanceEqualizerType(audio::format_int8_on_int16);
	performanceEqualizerType(audio::format_int16);
	performanceEqualizerType(audio::format_int16_on_int32);
	performanceEqualizerType(audio::format_int32);
	performanceEqualizerType(audio::format_int32_on_int64);
	performanceEqualizerType(audio::format_int64);
}

int main(int _argc, const char** _argv) {
	// the only one init for etk:
	etk::init(_argc, _argv);
	std::string inputName = "";
	std::string outputName = "output.raw";
	bool performance = false;
	bool perf = false;
	int64_t sampleRateIn = 48000;
	int64_t sampleRateOut = 48000;
	int32_t nbChan = 1;
	int32_t quality = 4;
	std::string test = "";
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (etk::start_with(data,"--in=")) {
			inputName = &data[5];
		} else if (etk::start_with(data,"--out=")) {
			outputName = &data[6];
		} else if (data == "--performance") {
			performance = true;
		} else if (data == "--perf") {
			perf = true;
		} else if (etk::start_with(data,"--test=")) {
			data = &data[7];
			sampleRateIn = etk::string_to_int32_t(data);
		} else if (etk::start_with(data,"--sample-rate-in=")) {
			data = &data[17];
			sampleRateIn = etk::string_to_int32_t(data);
		} else if (etk::start_with(data,"--sample-rate-out=")) {
			data = &data[18];
			sampleRateOut = etk::string_to_int32_t(data);
		} else if (etk::start_with(data,"--nb=")) {
			data = &data[5];
			nbChan = etk::string_to_int32_t(data);
		} else if (etk::start_with(data,"--quality=")) {
			data = &data[10];
			quality = etk::string_to_int32_t(data);
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_PRINT("Help : ");
			APPL_PRINT("    ./xxx --fb=file.raw --mic=file.raw");
			APPL_PRINT("        --in=YYY.raw            input file");
			APPL_PRINT("        --out=zzz.raw           output file");
			APPL_PRINT("        --performance           Generate signal to force algo to maximum process time");
			APPL_PRINT("        --perf                  Enable performence test (little slower but real performence test)");
			APPL_PRINT("        --test=XXXX             some test availlable ...");
			APPL_PRINT("            EQUALIZER          Test resampling data 16 bit mode");
			APPL_PRINT("                --sample-rate=XXXX   Input signal sample rate (default 48000)");
			
			exit(0);
		}
	}
	// PERFORMANCE test only ....
	if (performance == true) {
		performanceEqualizer();
		return 0;
	}
	if (test == "EQUALIZER") {
		/*
		APPL_INFO("Start resampling test ... ");
		if (inputName == "") {
			APPL_ERROR("Can not Process missing parameters...");
			exit(-1);
		}
		APPL_INFO("Read input:");
		std::vector<int16_t> inputData = etk::FSNodeReadAllDataType<int16_t>(inputName);
		APPL_INFO("    " << inputData.size() << " samples");
		// resize output :
		std::vector<int16_t> output;
		output.resize(inputData.size()*sampleRateOut/sampleRateIn+5000, 0);
		// process in chunk of 256 samples
		int32_t blockSize = 256*nbChan;
		
		Performance perfo;
		audio::algo::speex::Resampler algo;
		algo.init(nbChan, sampleRateIn, sampleRateOut, quality);
		int32_t lastPourcent = -1;
		size_t outputPosition = 0;
		for (int32_t iii=0; iii<inputData.size()/blockSize; ++iii) {
			if (lastPourcent != 100*iii / (inputData.size()/blockSize)) {
				lastPourcent = 100*iii / (inputData.size()/blockSize);
				APPL_INFO("Process : " << iii*blockSize << "/" << int32_t(inputData.size()/blockSize)*blockSize << " " << lastPourcent << "/100");
			} else {
				APPL_VERBOSE("Process : " << iii*blockSize << "/" << int32_t(inputData.size()/blockSize)*blockSize);
			}
			size_t availlableSize = (output.size() - outputPosition) / nbChan;
			perfo.tic();
			algo.process(&output[outputPosition], availlableSize, &inputData[iii*blockSize], blockSize, audio::format_int16);
			if (perf == true) {
				perfo.toc();
				usleep(1000);
			}
			outputPosition += availlableSize*nbChan;
		}
		if (perf == true) {
			APPL_INFO("Performance Result: ");
			APPL_INFO("    blockSize=" << blockSize << " sample");
			APPL_INFO("    min=" << perfo.getMinProcessing().count() << " ns");
			APPL_INFO("    max=" << perfo.getMaxProcessing().count() << " ns");
			APPL_INFO("    avg=" << perfo.getTotalTimeProcessing().count()/perfo.getTotalIteration() << " ns");
			
			APPL_INFO("    min=" << (float((perfo.getMinProcessing().count()*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
			APPL_INFO("    max=" << (float((perfo.getMaxProcessing().count()*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
			APPL_INFO("    avg=" << (float(((perfo.getTotalTimeProcessing().count()/perfo.getTotalIteration())*sampleRateIn)/blockSize)/1000000000.0)*100.0 << " %");
		}
		etk::FSNodeWriteAllDataType<int16_t>(outputName, output);
		*/
	}
	
}
