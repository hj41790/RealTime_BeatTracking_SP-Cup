
class Decimator {

	public:
		Decimator();
		~Decimator();

		void   initialize(double   decimatedSampleRate,
				double   passFrequency,
				unsigned oversampleRatio);

		double oversampleRate()  const { return mOversampleRate; }
		int    oversampleRatio() const { return mRatio; }

		void   decimate(float *in, float *out, size_t outCount);
		// N.B., input must have (ratio * outCount) samples.

	private:
		double mDecimatedSampleRate;
		double mOversampleRate;
		int    mRatio;              // oversample ratio
		float *mKernel;
		size_t mKernelSize;
		float *mShift;              // shift register
		size_t mCursor;

};

