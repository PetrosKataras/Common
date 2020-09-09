#version 150

#include "../common/offset.glsl"

#define saturate(s) clamp( s, 0.0, 1.0 )

in VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexIn;

uniform sampler2D uInputSource;
uniform sampler2D uNearSource;
uniform sampler2D uBlurSource;
uniform vec2      uInputSourceInvSize;
uniform float     uFarRadiusRescale = 3.0;
uniform int		  uDebugOption = 0;

uniform float uAperture = 1.0;
uniform float uFocalPlane = 5.0;
uniform float uFocalLength = 1.0;
uniform float uZnear = 1.0;
uniform float uZfar = 100;
vec3 result;
out vec4 fragColor;

const vec2 kCocReadScaleBias = vec2(2.0, -1.0);

// Boost the coverage of the near field by this factor.  Should always be >= 1
//
// Make this larger if near-field objects seem too transparent
//
// Make this smaller if an obvious line is visible between the near-field blur and the mid-field sharp region
// when looking at a textured ground plane.
const float kCoverageBoost = 1.0;

void main( void )
{
    ivec2 A      = ivec2( gl_FragCoord.xy - uOffset );
    vec4 pack    = texelFetch( uInputSource, A, 0 );
    vec3 sharp   = pack.rgb;
    vec4 blurred = texture( uBlurSource, ( gl_FragCoord.xy ) * uInputSourceInvSize );
	float CoC = blurred.a;
    vec4 near    = texture( uNearSource, ( gl_FragCoord.xy ) * uInputSourceInvSize );

    // Signed, normalized radius of the circle of confusion.
    // |normRadius| == 1.0 corresponds to camera->maxCircleOfConfusionRadiusPixels()
    float normRadius = CoC * kCocReadScaleBias.x + kCocReadScaleBias.y;

    // Fix the far field scaling factor so that it remains independent of the 
    // near field settings
    normRadius *= ( normRadius < 0.0 ) ? uFarRadiusRescale : 1.0;

    // Boost the blur factor
    normRadius = clamp( normRadius * 2.0, -1.0, 1.0 );

    // Decrease sharp image's contribution rapidly in the near field
    // (which has positive normRadius)
    if ( normRadius > 0.1 ) {
       normRadius = min( normRadius * 1.5, 1.0 );
    }

    if (kCoverageBoost != 1.0) {
        float a = saturate( kCoverageBoost * near.a );
        near.rgb = near.rgb * ( a / max( near.a, 0.001 ) );
        near.a = a;
    }

    // Two mixs, the second of which has a premultiplied alpha
    result = mix( sharp, blurred.rgb, abs( normRadius ) ) * ( 1.0 - near.a ) + near.rgb;
    
    /////////////////////////////////////////////////////////////////////////////////
    // Debugging options:
    const int SHOW_COC          = 1;
    const int SHOW_REGION       = 2;
    const int SHOW_NEAR         = 3;
    const int SHOW_BLURRY       = 4;
    const int SHOW_INPUT        = 5;
    const int SHOW_MID_AND_FAR  = 6;
    const int SHOW_SIGNED_COC   = 7;

    switch (uDebugOption) {
    case SHOW_COC:
        // Go back to the true radius, before it was enlarged by post-processing
        result.rgb = vec3( abs( CoC * kCocReadScaleBias.x + kCocReadScaleBias.y ) );
        break;

    case SHOW_SIGNED_COC:
        {
            // Go back to the true radius, before it was enlarged by post-processing
            float r = CoC * kCocReadScaleBias.x + kCocReadScaleBias.y;
            if (r < 0) {
                result.rgb = vec3( 0.0, 0.14, 0.8 ) * abs(r);
            } else {
                result.rgb = vec3( 1.0, 1.0, 0.15 ) * abs(r);
            }
        }
        break;

    case SHOW_REGION:
        if ( CoC < 0.49 )
        {
            // Far field: Dark blue
            result.rgb = vec3( 0.0, 0.07, 0.4 ) * ( dot( sharp, vec3( 1.0 / 3.0 ) ) * 0.7 + 0.3 );
        } else if ( CoC <= 0.51 ) {
            // Mifield: Gray
            result.rgb = vec3( 0.4 ) * ( dot( sharp, vec3( 1.0 / 3.0 ) ) * 0.7 + 0.3 );
        } else {
            result.rgb = vec3( 1.0, 1.0, 0.15 ) * ( dot( sharp, vec3( 1.0 / 3.0 ) ) * 0.7 + 0.3 );
        }
        break;

    case SHOW_BLURRY:
        result = blurred.rgb;
        break;

    case SHOW_NEAR:
        result = near.rgb;
        break;

    case SHOW_INPUT:
        result = sharp;
        break;

    case SHOW_MID_AND_FAR:
        // Just mix based on this pixel's blurriness. Works well in the background, less well in the foreground
        result = mix( sharp, blurred.rgb, abs( normRadius ) );
        break;
    }
	fragColor = vec4( result.rgb, 1.0 );
}
