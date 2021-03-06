#pragma once

#include <vector>
#include <sstream>
#include <assert.h>


class BitBuffer
{
protected:
    unsigned short currentNum;
    unsigned char currentChar;
    std::stringbuf *buffer;

    BitBuffer(std::stringbuf *inBuf) : buffer(inBuf) {}

public:

    std::stringbuf* GetBuffer(){ return buffer; }

    void Flush()
    {
        currentChar <<= 8 - currentNum;
        currentNum = 0;
        buffer->sputc(currentChar);
        currentChar = 0;
    }

    void PutBit(unsigned short bit)
    {
        assert(bit < 2);
        assert(currentNum < 8);

        currentChar <<= 1;
        currentChar += bit;

        currentNum++;

        if(currentNum == 8)
        {
            Flush();
        }
    }

    bool GetBit()
    {
        if(currentNum == 8)
        {
            currentNum = 0;
            currentChar = buffer->sbumpc();
        }

        return (currentChar & (1<<(7 - currentNum++))) != 0;
    }
};

class InBitBuffer : public BitBuffer
{
public:
    InBitBuffer(std::stringbuf *inBuf) : BitBuffer(inBuf)
    {
        currentNum = 0;
        currentChar = buffer->sbumpc();
    }
};

class OutBitBuffer : public BitBuffer
{
public:
    OutBitBuffer(std::stringbuf *inBuf) : BitBuffer(inBuf)
    {
        currentNum = 0;
        currentChar = 0;
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

    void SetFrequencyFromBuffer(std::stringbuf &inBuf)
    {
        std::streamsize size = inBuf.str().size();
        cumulativeFrequency = std::vector<CodeValue>(257, 0);

        for(int i = 0; i < size; i++)
        {
            unsigned char c;
            c = inBuf.sbumpc();

            for(int j = c + 1; j < 257; j++)
            {
                cumulativeFrequency[j]++;
                assert(cumulativeFrequency[j] <= MAX_FREQ);
            }
        }

        inBuf.pubseekpos(0);
    }

    void GetFrequencyFromBuffer(std::stringbuf &inBuf)
    {
        cumulativeFrequency = std::vector<CodeValue>(257, 0);

        for(int i = 0; i < 257; i++)
        {
            inBuf.sgetn((char*)(&cumulativeFrequency[i]), sizeof(CodeValue));
        }
    }

public:

    void Encode(std::stringbuf &outBuffer, std::stringbuf &inBuffer, unsigned int numValues)
    {
        inBuffer.pubseekpos(0);

        SetFrequencyFromBuffer(inBuffer);

        CodeValue high = MAX_CODE;
        CodeValue low = 0;
        CodeValue pendingBits = 0;

        outBuffer.sputn((char*)(&numValues), sizeof(unsigned int));

		for(unsigned int i = 0; i < cumulativeFrequency.size(); i++)
		{
			outBuffer.sputn((char*)(&cumulativeFrequency[i]), sizeof(CodeValue));
        }

        OutBitBuffer bitBuffer(&outBuffer);

        unsigned int i = 0;
        for(; i < numValues; i++)
        {
            assert(low < high);
            assert(high <= MAX_CODE);

            assert(inBuffer.in_avail() != 0);
            unsigned char value = inBuffer.sbumpc();

            CodeValue range = high - low + 1;
            Probability prob = GetProbability(value);

            high = low + (range * prob.upper / prob.denominator) - 1;
            low  = low + (range * prob.lower / prob.denominator);

            for(;;)
            {
                if(high < ONE_HALF)
                {
                    OutputBit(0, pendingBits, bitBuffer);
                }
                else if(low >= ONE_HALF)
                {
                    OutputBit(1, pendingBits, bitBuffer);
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

        assert(inBuffer.in_avail() == 0);

        pendingBits++;
        if(low < ONE_FOURTH)
        {
            OutputBit(0, pendingBits, bitBuffer);
        }
        else
        {
            OutputBit(1, pendingBits, bitBuffer);
        }
        assert(pendingBits == 0);

        bitBuffer.Flush();
    }

    void Decode(std::stringbuf &inBuffer, std::stringbuf &outBuffer)
    {
        inBuffer.pubseekpos(0);

        unsigned int numValues;

        inBuffer.sgetn((char*)(&numValues), sizeof(unsigned int));

        GetFrequencyFromBuffer(inBuffer);

        InBitBuffer bitBuffer(&inBuffer);

        CodeValue low = 0;
        CodeValue high = MAX_CODE;

        CodeValue value = 0;

        for(int i = 0; i < CODE_VALUE_BITS; i++)
        {
            value <<= 1;
            value += bitBuffer.GetBit() ? 1 : 0;
        }        

        CodeValue count = GetCount();

        for(unsigned int i = 0;; i++)
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
            if(i == numValues - 1)
            {
                break;
            }

            for(;;)
            {
                if(high < ONE_HALF)
                {
                    ;
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
                value += bitBuffer.GetBit() ? 1 : 0;
            }
        }
    }
};