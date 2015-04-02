#include <vector>
#include <sstream>
#include <assert.h>

class BitBuffer
{
private:
    unsigned short currentNum;
    unsigned char currentChar;
    std::stringbuf buffer;

public:
    BitBuffer() : currentChar(0), currentNum(0)
    {
        buffer.pubseekpos(0, std::ios_base::in);
    }
    
    BitBuffer(std::string &inString) : currentChar(0), currentNum(0)
    {
        std::streamsize num = buffer.sputn(inString.c_str(), inString.size());
        buffer.pubseekpos(0, std::ios_base::out);

        currentChar = buffer.sbumpc();
    }

    void Flush()
    {
        currentChar <<= currentNum;
        buffer.sputc(currentChar);
        currentChar = 0;
    }

    std::stringbuf& GetBuffer(){return buffer;}    

    void PutBit(unsigned short bit)
    {
        assert(bit < 2);
        assert(currentNum < 8);

        currentChar <<= 1;
        currentChar += bit;

        currentNum++;

        if(currentNum == 8)
        {
            currentNum = 0;
            Flush();
        }
    }

    bool GetBit()
    {
        if(currentNum == 8)
        {
            currentNum = 0;
            currentChar = buffer.sbumpc();
        }

        return (currentChar & (1<<(7 - currentNum++))) != 0;
    }
};

template<   typename CodeValue_ = unsigned long,
            int CODE_VALUE_BITS_ = (std::numeric_limits<CodeValue_>::digits + 3) / 2,
            int FREQUENCY_BITS_ = std::numeric_limits<CodeValue_>::digits - CODE_VALUE_BITS_>            
class ArithmeticEncoder
{
    typedef CodeValue_ CodeValue;

    struct Probability
    {
        CodeValue lower;
        CodeValue upper;
        CodeValue denominator;
    };

private:
    static const int CODE_VALUE_BITS    = CODE_VALUE_BITS_;
    static const int FREQUENCY_BITS     = FREQUENCY_BITS_;
    static const CodeValue MAX_CODE     = (CodeValue(1) << CODE_VALUE_BITS) - 1;
    static const CodeValue MAX_FREQ     = (CodeValue(1) << FREQUENCY_BITS ) - 1;
    static const CodeValue ONE_FOURTH   = CodeValue(1) << (CODE_VALUE_BITS - 2);
    static const CodeValue ONE_HALF     = ONE_FOURTH * 2;
    static const CodeValue THREE_FOURTHS= ONE_FOURTH * 3;

    std::vector<CodeValue> cumulativeFrequency;

    Probability GetProbability(unsigned char c)
    {
        Probability prob = {cumulativeFrequency[c], cumulativeFrequency[c + 1], cumulativeFrequency.back()};
        return prob;
    }

    Probability GetChar(CodeValue scaledValue, unsigned char &c)
    {
        for(unsigned int i = 0; i < cumulativeFrequency.size(); i++)
        {
            if(scaledValue < cumulativeFrequency[i+1])
            {
                c = i;
                Probability p = {cumulativeFrequency[i], cumulativeFrequency[i+1], cumulativeFrequency.back()};
                return p;
            }
        }
        throw std::logic_error("scaledValue higher than cumulativeFrequency");
    }

    void OutputBit(unsigned short bit, CodeValue &pendingBits, BitBuffer &buffer)
    {
        buffer.PutBit(bit);

        unsigned short reverseBit = 1 - bit;
        while(pendingBits)
        {
            buffer.PutBit(reverseBit);
            pendingBits--;
        }    
    }

    CodeValue GetCount()
    {
        return cumulativeFrequency.back();
    }

public:
    void SetFrequencyFromDistribution(std::vector<unsigned int> &distribution)
    {
        cumulativeFrequency = std::vector<CodeValue>(distribution.size() + 1, 0);

        for(unsigned int i = 0; i < distribution.size(); i++)
        {
            CodeValue currentVal = distribution[i];
            for(unsigned j = i + 1; j < cumulativeFrequency.size(); j++)
            {
                cumulativeFrequency[j] += currentVal;
                assert(cumulativeFrequency[j] <= MAX_FREQ);
            }
        }
    }

    void Encode(BitBuffer &outBuffer, std::stringbuf &inBuffer, unsigned int numValues)
    {
        CodeValue high = MAX_CODE;
        CodeValue low = 0;
        CodeValue pendingBits = 0;

        unsigned int i = 0;
        for(; i < numValues; i++)
        {
            assert(low < high);
            assert(high <= MAX_CODE);

            unsigned char value = inBuffer.sbumpc();

            CodeValue range = high - low + 1;
            Probability prob = GetProbability(value);

            high = low + (range * prob.upper / prob.denominator) - 1;
            low  = low + (range * prob.lower / prob.denominator);

            for(;;)
            {
                if(high < ONE_HALF)
                {
                    OutputBit(0, pendingBits, outBuffer);
                }
                else if(low >= ONE_HALF)
                {
                    OutputBit(1, pendingBits, outBuffer);
                }
                else if(low >= ONE_FOURTH && high < THREE_FOURTHS)
                {
                    pendingBits++;
                    low  -= ONE_FOURTH;
                    high -= ONE_FOURTH;
                }
                else
                {
                    break;
                }

                high <<= 1;
                high++;
                low <<= 1;
                high &= MAX_CODE;
                low  &= MAX_CODE;
            }
        }

        pendingBits++;
        if(low < ONE_FOURTH)
        {
            OutputBit(0, pendingBits, outBuffer);
        }
        else
        {
            OutputBit(1, pendingBits, outBuffer);
        }
        assert(pendingBits == 0);

        outBuffer.Flush();
    }

    void Decode(BitBuffer &inBuffer, std::stringbuf &outBuffer, unsigned int numValues)
    {
        CodeValue low = 0;
        CodeValue high = MAX_CODE;

        CodeValue value = 0;

        unsigned short bitCount = 0;
        unsigned int byteCount = 0;

        for(int i = 0; i < CODE_VALUE_BITS; i++)
        {
            value <<= 1;
            value += inBuffer.GetBit() ? 1 : 0;

            bitCount++;
            if(bitCount == 8)
            {
                bitCount = 0;
                byteCount++;
            }
        }        

        CodeValue count = GetCount();

        for(unsigned int i = 0; i < numValues; i++)
        {
            CodeValue range = high - low + 1;
            CodeValue scaledValue = ((value - low + 1) * count - 1) / range;

            assert(scaledValue <= count);
            assert(low <= high);
            assert(high <= MAX_CODE);

            unsigned char c;
            Probability prob = GetChar(scaledValue, c);

            high = low + (range * prob.upper / prob.denominator) - 1;
            low  = low + (range * prob.lower / prob.denominator);

            outBuffer.sputc(c);

            for(;;)
            {
                if(high < ONE_HALF)
                {
                    
                }
                else if(low >= ONE_HALF)
                {
                    value -= ONE_HALF;
                    low   -= ONE_HALF;
                    high  -= ONE_HALF;
                }
                else if(low >= ONE_FOURTH && high < THREE_FOURTHS)
                {
                    value -= ONE_FOURTH;
                    low   -= ONE_FOURTH;
                    high  -= ONE_FOURTH;
                }
                else
                {
                    break;
                }

                low  <<= 1;
                high <<= 1;
                high++;
                value <<= 1;
                value += inBuffer.GetBit() ? 1 : 0;

                bitCount++;
                if(bitCount == 8)
                {
                    bitCount = 0;
                    byteCount++;
                }
            }
        }

        std::cout<<"decode bytes read: "<<byteCount<<std::endl;
    }
};