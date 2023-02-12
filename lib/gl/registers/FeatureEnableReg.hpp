#ifndef _FEATURE_ENABLE_REG_
#define _FEATURE_ENABLE_REG_

namespace rr
{

class FeatureEnableReg
{
public:
    FeatureEnableReg() = default;
    void setEnableFog(const bool val) { m_regVal.fields.fog = val; }
    void setEnableBlending(const bool val) { m_regVal.fields.blending = val; }
    void setEnableDepthTest(const bool val) { m_regVal.fields.depthTest = val; }
    void setEnableAlphaTest(const bool val) { m_regVal.fields.alphaTest = val; }
    void setEnableTmu(const uint8_t tmu, const bool val) { if (tmu == 0) m_regVal.fields.tmu0 = val; else m_regVal.fields.tmu1 = val; }
    void setEnableScissor(const bool val) { m_regVal.fields.scissor = val; }

    bool getEnableFog() const { return m_regVal.fields.fog; }
    bool getEnableBlending() const { return m_regVal.fields.blending; }
    bool getEnableDepthTest() const { return m_regVal.fields.depthTest; }
    bool getEnableAlphaTest() const { return m_regVal.fields.alphaTest; }
    bool getEnableTmu(const uint8_t tmu) const { return (tmu == 0) ? m_regVal.fields.tmu0 : m_regVal.fields.tmu1; }
    bool getEnableScissor() const { return m_regVal.fields.scissor; }

    uint32_t serialize() const { return m_regVal.data; }
    static constexpr uint32_t getAddr() { return 0x0; }
private:
    union RegVal
    {
        #pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : fog(false)
                , blending(false)
                , depthTest(false)
                , alphaTest(false)
                , tmu0(false)
                , scissor(false)
                , tmu1(false)
            { }

            uint32_t fog : 1;
            uint32_t blending : 1;
            uint32_t depthTest : 1;
            uint32_t alphaTest : 1;
            uint32_t tmu0 : 1;
            uint32_t scissor : 1;
            uint32_t tmu1 : 1;
        } fields {};
        uint32_t data;
        #pragma pack(pop)
    } m_regVal;
};

} // namespace rr

#endif // _FEATURE_ENABLE_REG_
