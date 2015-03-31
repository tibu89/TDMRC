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
    BitBuffer() : currentChar(0), currentNum(0){}

    void Flush(){ buffer.sputc(currentChar);}

    std::stringbuf& GetBuffer(){return buffer;}

    void PutBit(unsigned short bit)
    {
        assert(bit < 2);

        currentChar <<= 1;
        currentChar |= bit;
        currentNum++;

        if(currentNum == 8)
        {
            currentNum = 0;
            Flush();
        }
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
    CodeValue high;
    CodeValue low;
    CodeValue pendingBits;

    Probability GetProbability(unsigned char c)
    {
        Probability prob = {cumulativeFrequency[c], cumulativeFrequency[c + 1], cumulativeFrequency.back()};
        return prob;
    }

    void OutputBit(unsigned short bit, BitBuffer &buffer)
    {
        buffer.PutBit(bit);

        unsigned short reverseBit = 1 - bit;
        while(pendingBits)
        {
            buffer.PutBit(reverseBit);
            pendingBits--;
        }    
    }

public:
    ArithmeticEncoder() : high(MAX_CODE), low(0), pendingBits(0) {}

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

    void EncodeValue(unsigned char value, BitBuffer &outBuffer)
    {
        CodeValue range = high - low + 1;
        Probability prob = GetProbability(value);

        high = low + (range * prob.upper) / prob.denominator - 1;
        low  = low + (range * prob.lower) / prob.denominator;

        for(;;)
        {
            if(high < ONE_HALF)
            {
                OutputBit(0, outBuffer);
            }
            else if(low >= ONE_HALF)
            {
                OutputBit(1, outBuffer);
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
};