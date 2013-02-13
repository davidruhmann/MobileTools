/* File:	Missing.h
 * Created: Dec 2011
 * Author:	David Ruhmann
 *
 * Copyright (c) 2011 David Ruhmann
 */

/* Purpose:
 * These are some definitions that Microsoft has forgotten to include in their
 *  library for Windows CE \ Mobile.
 */

//// Definitions.
#ifndef HGDI_ERROR
#define HGDI_ERROR ((HANDLE)(0xFFFFFFFFL))
#endif

//// Enumerations (Not Current Used).
enum EncoderValue
{
   EncoderValueColorTypeCMYK,             // 0
   EncoderValueColorTypeYCCK,             // 1
   EncoderValueCompressionLZW,            // 2
   EncoderValueCompressionCCITT3,         // 3
   EncoderValueCompressionCCITT4,         // 4
   EncoderValueCompressionRle,            // 5
   EncoderValueCompressionNone,           // 6
   EncoderValueScanMethodInterlaced,      // 7
   EncoderValueScanMethodNonInterlaced,   // 8
   EncoderValueVersionGif87,              // 9
   EncoderValueVersionGif89,              // 10
   EncoderValueRenderProgressive,         // 11
   EncoderValueRenderNonProgressive,      // 12
   EncoderValueTransformRotate90,         // 13
   EncoderValueTransformRotate180,        // 14
   EncoderValueTransformRotate270,        // 15
   EncoderValueTransformFlipHorizontal,   // 16
   EncoderValueTransformFlipVertical,     // 17
   EncoderValueMultiFrame,                // 18
   EncoderValueLastFrame,                 // 19
   EncoderValueFlush,                     // 20
   EncoderValueFrameDimensionTime,        // 21
   EncoderValueFrameDimensionResolution,  // 22
   EncoderValueFrameDimensionPage         // 23
};

typedef enum
{
  EncoderParameterValueTypeByte            = 1,
  EncoderParameterValueTypeASCII           = 2,
  EncoderParameterValueTypeShort           = 3,
  EncoderParameterValueTypeLong            = 4,
  EncoderParameterValueTypeRational        = 5,
  EncoderParameterValueTypeLongRange       = 6,
  EncoderParameterValueTypeUndefined       = 7,
  EncoderParameterValueTypeRationalRange   = 8,
  EncoderParameterValueTypePointer         = 9 
} EncoderParameterValueType;
