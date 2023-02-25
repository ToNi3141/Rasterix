/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "IceGL.hpp"
#include "RendererArduino.hpp"
#include "RenderConfigs.hpp"
#include "gl.h"
#include "glu.h"

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <hardware/dma.h>

#include <algorithm>

// Simple BusConnector which wraps the SPI
class BusConnector : public rr::IBusConnector
{
public:
    BusConnector() { }

    virtual ~BusConnector() = default;

    virtual void writeData(const std::span<const uint8_t>& data) override 
    {
        dma_channel_config c = dma_channel_get_default_config(dma_tx);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
        dma_channel_configure(dma_tx, &c,
                        &spi_get_hw(spi_default)->dr, // write address
                        data.data(), // read address
                        data.size(), // element count (each element is of size transfer_data_size)
                        true); // Start immediately
    }

    virtual bool clearToSend() override 
    {
        return !dma_channel_is_busy(dma_tx);
    }

    void init()
    {
        spi_init(spi_default, 48 * 1000 * 1000);
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

        // Make the SPI pins available to picotool
        bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
        // Make the CS pin available to picotool
        bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        // Grab some unused dma channels
        dma_tx = dma_claim_unused_channel(true);
    }

    uint dma_tx;
};

static constexpr bool ENABLE_LIGHT = true;
static constexpr bool ENABLE_BLACK_WHITE = false;

GLuint m_textureId = 0;

static const uint32_t RESOLUTION_H = 240;
static const uint32_t RESOLUTION_W = 320;
BusConnector m_busConnector;
rr::RendererArduino<rr::RenderConfigPico> m_renderer{m_busConnector};


static constexpr uint16_t cubeIndex[] = {
    0, 1, 2, 0, 2, 3, // Face three
    4, 5, 6, 4, 6, 7, // Face five
    8, 9, 10, 8, 10, 11, // Face two
    12, 13, 14, 12, 14, 15, // Face six
    16, 17, 18, 16, 18, 19, // Face one
    20, 21, 22, 20, 22, 23 // Face four
};

static constexpr float cubeVerts[] = {
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,

    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
};

static constexpr float cubeTexCoords[] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
};

static constexpr float cubeNormals[] = {
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,

    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,

    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,

    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,

    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
};

// Picture exported from GIMP
#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}
static char *cubeTexture =
	"B)G@G*WTGJ_VE:;MFJSSG:_VD*3JA)C>AY[EE*OR>Y/>BZ/N@YWKEK#^D:K[:H+-"
	"4V:I05.07G\"O:7NZ-DB),4.$5VBM97:[7W\"W9'6\\7V^[;7W);7W+566S0$^?(S1["
	",D\"&.TF/1%*88G\"VE*7JK+X\"F:SP@I79?Y/9AIKBBZ'KE:OUG+3_B:#MDJGX>8_9"
	"?I'27G\"M35^>2ER;1%:7&\"IK0%&587*W:'F`9'6\\4F*L5&2P566S3U^M5F6U4%ZF"
	"!!!1%2!C(2QO)3!S+3I_/4J/6&>K>8C,?Y#5>(G.CJ#GB)KAG*[W9WG\">X[9=(;-"
	"=(;';GZ[9'.S5F6E87\"R!Q98,4\"$?(K0:WG!;7O#7VVW86^Y8F^\\8&VZ;GO*:WG#"
	"45F;14^.,CI\\)S!R*3)T)3!S+#=Z/DN.5&&D4E^D<8#$>8?-GJSR>(;,@9+9?Y#5"
	"D)_?B)C5CY[><X+\"9G6W$R)D(C%U=X;*:7>]<7_':':^;7O%;GS&:7;#;GO(9'*\\"
	"3U253E:545:755V=55V=.T2&(RQN(BMM0$F+.T:)-T*%0$J08W\"U>(7*?HO0C)G<"
	"I++P;7NX:WFW3ER:0$Z..$:&)3)U4%VB;'F^;GO!7&FO7&BQ7FJS7FJU97&\\6&2O"
	"96VM1T^/+S=W1DZ.<7F[<7J\\25*6*3)V'RAL,CU`+SA\\15\"38&NNDIW@A(_2FJ7F"
	"BYG74V&>5F2B1U63/DR,6FBH/4J--D.&9G.X:G>\\6F>M6F>M7FJS8&RU97&Z56&J"
	"?H?)>H/%;'6Y8VZQ:W6[<'J`>(+)AI#7BI3;@8[42%*9%R1I!!!3(B]T)S1Y9'&T"
	"B935AY+1D)O:A(_.>(/$8FVN4UZA*C5X7&>J:G6X9&ZT9W&W:7.Z:'*Y:W6\\6F2M"
	";GW!<X'';'J`9W6];WW%<H#*<7[+=H/0BYCEG:OUB9?A6VFQ+#J\")3-Y%B1J5&.G"
	";GR\\E:#?E)_@C9C9F:3E56\"A7VJM)C%T5V*E9G&T7VFO7&:L6F2K7&:M:7.Z8&JQ"
	"=XO3:'S$:'O&<(/0<X;4?9#>BYWND*+S@I3E>HS=BY[LD*/PH;3_B)SD>X_7:7R`"
	"6FFK6&:F66:I?8K-B9;97VRO9'&T-T2',C^$>(7*8&VR5F.H97*W6&6J8&VS6VBN"
	"9H#+7';!7WG%97_-97S-;8/7?)+G@YGN>8_D=8O?CJ/UKL03FJ_]>Y'=>([8CJ+H"
	"A9382%687FNP<X##=('&8F^R4E^D)S1Y&\"5J97*W7VRQ7VRQ;'F^6VBM7VRR66:K"
	";XO6:(/1;(?7;(?99'W19G[5<HKC=H[G@9GRC*3[BJ#UCJ3X@)?FJL(.GK;_>(_6"
	"AYC=)#-W3ERB1U::/$J046\"D2%:<.DB.*SE_:7>]8&ZT66>M8&ZT4V&G6VFO6VJN"
	"?)?E<Y#==I'A=9#B;8?=;X?@=H[G=(SGCJ/_A)GS@ICMAYWQ<8;6A)KFBJ#J@97="
	"J+G`2URC7&JR-4:-,3^'35ZE1E2<05*96VFQ?(W2:7>_56:K5V6M2URA45^G46\"D"
	"=I#<:X?2;(;4;XC9;H?;=HOC>8[H;X3>>HWH;H+:?8_D?Y'BBY[KC:'IE*;M?I#7"
	"/E\"9%RER#1UG!!!8*3F#76^X=87/?8_83%RF6&JQ156?-TF0/DZ8-4>.*SN%(#%V"
	">)#9;87.:8#-:8#/:7[0<(3:<8+;8W3-A)3K@I+H?HWA8&^_<H+,87*W0E&5\"!E="
	"%BAQ0E6@8G/!9GG$AICC=HG4<(+-9WK%;H#+97G!4V6N4&2L9':_:W_'87.\\56>N"
	"?)#6=(C.<83/;8#-:WS-<8'5<X'89G3+>X?<>878>X;7@H[;>H3+1$^20$N,<X##"
	"B9ODC*+NEZKX@ICD?(_<8GC$=8C5@);B=(?487?!5VJU7'*\\>X[9C*+LA)?B>HS5"
	"<('%;8#$<(/';X'(;G[*>8G7@Y#@>H?6?XO8BI;A9&ZW66*H3%240DN'14Z(AY'/"
	"O,T6?9#>@Y;D@)+C?I/C;X34:X#08'7%56JZ26\"O4F>U97K(@ICDCJ3P=HS89'?\""
	"5&.E0U667G\"Q:'J[>HO/?Y#4F*?KB9C:@([.8FZK-T%]/D=_0$=\\:G\"A5EJ)AHO#"
	"IK#WD9[N=X;8CY_S?8[E8W?/>8WE5&G#3V2^9W_67G3)<(;:J<$1?Y;E>(W;?9/?"
	"=H7%CIW=FJSK@Y74?([-@Y74CJ#?@Y/0#AY:!!!(#1A/#QE-'2-4/$!O5UF(8&:;"
	"8VRRGZKZAY3D=(/5@)#F:GO49'?2<(7?;H/==8WFA9[RFK$$:'_.>H_=C:/O>I#:"
	"CY[>AY;6D*+CH++SD*/DCJ'B;8#!.DR+!!11\"QM7!1%,!Q)+$1M1$AE.,SEL45B/"
	"2%*1:G>]@8_7AI;B?8[<97?(9GO-?9/G>([B9'O,<(?6DJ?U>I#:@I;><H;,<X?+"
	"G:SLE*/CFZWNDJ3E-DF,'C%T$B5H!!!3<H7(LL4%AY;67FRJ$1U:$AQ8$QQ6$QQ6"
	"3%:20DV,:WFY?HW1:WS!;X'*>8S9=(K6<8?3=8O7I+L$GK+ZI;G_AYK=?I'2H+7U"
	"G*OMB)?9E:;JDZ3H'C%U!!!4%RMQ25VCH+3ZG;'WLL4+T^4I.$>+!!!1!!!/\"152"
	"(\"MB.D=]4V*:35V97G\"OB)O<AIG=:G[$9'B^>X_5L\\@-D:3GBY[?FJSKG['NDJ;B"
	"A)/7>XS0'\"UQ!AE=<87+N=$7H[H#H[P$I;P'R-\\JG[+_@Y;AO<`8OL\\8;GS$'BMN"
	"\"!5+&B972UB-<H&Y?(S(DJ3AFJSM=8C+?I'5D*3H>X[1:'J[5FBG4V.@&2EE\"QU:"
	"3%VB\"!E>4F.JL,,+H+/^BJ#LK\\<3JL40L\\L9E*O\\O-0DLLH8B9[NM<D6O<\\=7F^V"
	")3)H\"Q=('\"E>7VZD6VNE/U&.5FBI3F&D05284V:J,T2(1%:71E65+SU[(2]L4V.@"
	"9'6\\K\\((I;?`;H',G;/_F;#]AJ#NDZS]@)GLD:K]G[D,H[T/>I#DGK0%B)WOH++]"
	"I;3NGZS?0$V#/4R$8'\"L,D2#.4J.:'F^76^V(3-Z#R!E'RYR,C^\"3%J:H:SKDZ+D"
	"K+\\'I;@\"HK0!E:CV@9;F@9CIC:;YD*K^@ISPC*;Z>9/ID:K^H[T0F:`#<8?;F*OX"
	"DJ#>I;+H@8W(1U63-42&0U28C9[E\\@0`S=XII[@#97.],#Z&&B=M<7O!C9;:>XC."
	"K;\\,J+P)IKD)EJO[?Y;G<XS?=8_C=8_CBJ3Z@9OQG[H-G[D,9X#3<HC<CZ;WFZ[["
	"C9S>J+;S5F2D!!!3-D>.KL$)RMPIJ;L(H;#`S-PKH:[]:G?$GZOTEZ'HL;K`HJ[W"
	"DJ3WE*G[EZO`CZ7YA)KO?9;J@9OQAJ#V?ICL?YGMB*'T?I?HE:S[F:[^EJOYL,0."
	"G:OQ/DV197.[R]PEP]0AEZCWEZCYE*;YBYOOK;\\/J[L*A9+AKKL%P\\X6AH[6E*#K"
	"EJK`FK$%EJP#BJ+YA9WT@9GPA)[TCZG_<XWA<XS?D*GZ:8#-<(;2D:3ODZ7N<X7."
	"*SR#-D>.FJKVOL`=?(W>BISOKL$5=(;;FJT!D*#TK;\\-GJ[\\AY7?LK\\'FZ7LG:GR"
	"F*X'H+8/EZT&BZ/\\C:7^A9_W@9OQBZ7YA)WP7'7&=8S9A)KDI[G`<X;*0%&57F^S"
	"W.\\US^(LL\\42D:3R?8_@=XS>?)#E?Y/HAYSNGK$!M\\D6M<81EZ7M7VRQUN(DK;K_"
	"D*4#FK(/DZL(CZ@\"F;,-CZH!?9?M@YWQ9'W.BJ'NJ+\\(E*CN@Y38'\"MK&2EF[/P["
	"[P$`Q-@BEZKXBI_MD*7W@9CI=HS@@9CI=8K<I;L(K+`*J[X$E*7I/4R,>(;$;'JZ"
	"F*`.H[L9EJX,DJP(I+X8E[()?ICN@)OMDJSZG[@\"F:WS+T*%\"QM89G6OGJKCWNTI"
	"D*/G@9?AAIOI@9CE<HG:?9;GB:+S=Y#ABZ+QIKT(J;X%GK'UBYW<25F5%B5?+#IW"
	"B)S[E*D'B9[ZCZ3\\LLD=MLP=I[T,L,<2C*#H?9#41EF<#1]@9':U_```W>XIO,\\-"
	"@)3<?93A=X[=@YKIEJW^A9SM>(_@C:3UDJ?UI[L'K\\((E:;JGJ[K)S9P,#QU)C5O"
	"";


// returns a valid textureID on success, otherwise 0
GLuint loadTexture(const char* tex)
{
    static constexpr uint32_t WIDTH = 32;
    static constexpr uint32_t HEIGHT = 32;
    int level = 0;
    int border = 0;

    // data is aligned in byte order
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // request textureID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Convert from the GIMP format to RGB888
    char pixelBuffer[WIDTH * HEIGHT * 3];
    char *pixelBufferPtr = pixelBuffer;
    for (uint32_t i = 0; i < WIDTH * HEIGHT; i++)
    {
        HEADER_PIXEL(tex, pixelBufferPtr);
        pixelBufferPtr += 3;
    }

    // specify the 2D texture map
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, WIDTH, HEIGHT, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
    
    // define how to filter the texture
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // define clamping mode
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // return unique texture identifier
    return textureID;
}

void init()
{
    rr::IceGL::createInstance(m_renderer);
    m_renderer.setRenderResolution(RESOLUTION_W, RESOLUTION_H);

    m_textureId = loadTexture(cubeTexture);

    // Setup viewport, depth range, and projection matrix
    glViewport(0, 0, RESOLUTION_W, RESOLUTION_H);
    glDepthRange(0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (float)RESOLUTION_W/(float)RESOLUTION_H, 1.0, 111.0);

    // Enable depth test and depth mask
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Setup one light which will lighten the cube
    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 0.0 };
    GLfloat light_diffuse[] = { 1.5, 1.5, 1.5, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 1.0, 3.0, 6.0, 0.0 };

    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    if constexpr (ENABLE_LIGHT)
    {
       glActiveTexture(GL_TEXTURE0);
       glEnable(GL_LIGHT0);
       glEnable(GL_LIGHTING);

       static constexpr float colors[4] = {0.0, 0.0, 0.0, 0.0};
       glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
       glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    if constexpr (ENABLE_BLACK_WHITE)
    {
        static constexpr float colors[4] = {.7, .7, .7, 0.0};
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
    }

    glLineWidth(5.0f);
}

void draw()
{
    // Setup clear color and clear the framebuffer
    glDisable(GL_SCISSOR_TEST);
    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Fooling around with the scissor
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 50, 200, 46);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScissor(200, 50, 200, 48);
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScissor(400, 50, 200, 50);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glScissor(200, 200, 200, 100);

    // Setup the model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // set camera
    gluLookAt(12.0f, -2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    // Rotate cube
    static float t2 = 0;
    t2 += 1.0f;
    glRotatef(t2, 0.0f, 0.0f, 1.0f);

    // Scale the cube a bit to make it a bit bigger
    glScalef(1.5f, 1.5f, 1.5f);

    // Tex Coords for texture 0
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords);

    // Enable normals
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, cubeNormals);
    
    // Enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVerts);

    // Draw the cube
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndex) / sizeof(cubeIndex[0]), GL_UNSIGNED_SHORT, cubeIndex);
}

int main()
{
    stdio_init_all();
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    bool led = false;
    m_busConnector.init();
    init();
    while (1)
    {
        gpio_put(LED_PIN, led);
        led = !led;
        draw();
        rr::IceGL::getInstance().commit();
    }
}
