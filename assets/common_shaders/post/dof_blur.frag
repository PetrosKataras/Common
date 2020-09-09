#version 330 core

#define saturate(s) clamp( s, 0.0, 1.0 )

#if HORIZONTAL
    const ivec2 kDirection = ivec2(1, 0);
#else
    const ivec2 kDirection = ivec2(0, 1);
#endif
in VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexIn;

uniform vec2      uInputSourceInvSize;
uniform sampler2D uBlurSource;
uniform sampler2D uNearSource;
uniform sampler2D uDepthSource;

uniform int       uMaxCoCRadiusPixels;
uniform int       uNearBlurRadiusPixels;
uniform float     uInvNearBlurRadiusPixels;

uniform float uAperture = 1.0;
uniform float uFocalPlane = 5.0;
uniform float uFocalLength = 1.0;
uniform float uZnear = 1.0;
uniform float uZfar = 100;

layout(location = 0) out vec4 nearResult;
layout(location = 1) out vec4 blurResult;

float linearize(float depth)
{
	return (-uZfar * uZnear ) / ( depth * ( uZfar - uZnear ) - uZfar );
}

float linearizeAlt( float depth )
{
	return 2.0 *uZfar * uZnear / ( uZfar + uZnear - depth * ( uZfar - uZnear ) );
}

bool inNearField( in float radiusPixels ) {
    return radiusPixels > 0.10;
}

void main( void )
{

	const int KERNEL_TAPS = 6;
    float kernel[KERNEL_TAPS + 1];
    
    // 11 x 11 separated kernel weights.  This does not dictate the 
    // blur kernel for depth of field; it is scaled to the actual
    // kernel at each pixel.
	//   Custom disk-like // vs. Gaussian
    kernel[6] = 0.00;     // 0.00000000000000;  // Weight applied to outside-radius values
    kernel[5] = 0.50;     // 0.04153263993208;
    kernel[4] = 0.60;     // 0.06352050813141;
    kernel[3] = 0.75;     // 0.08822292796029;
    kernel[2] = 0.90;     // 0.11143948794984;
    kernel[1] = 1.00;     // 0.12815541114232;
    kernel[0] = 1.00;     // 0.13425804976814;
    
    // Accumulate the blurry image color
    blurResult.rgb  = vec3( 0.0 );
    float blurWeightSum = 0.0;
    
    // Accumulate the near-field color and coverage
    nearResult = vec4( 0.0 );
    float nearWeightSum = 0.0;

#if RUN_PRODUCTION
	ivec2 A = ivec2( gl_FragCoord.xy );
#else
	ivec2 A = ivec2( gl_FragCoord.xy * ( kDirection * 3 + ivec2( 1 ) ) );
#endif
	vec4 blurSource = texelFetch( uBlurSource, A, 0 );
#if HORIZONTAL
	vec4 depthSource = texelFetch( uDepthSource, A, 0 );
	float depth = linearizeAlt( depthSource.r );
	float CoC =  uAperture * ( ( ( depth - uFocalLength ) / uFocalPlane ) );
	CoC = clamp( CoC, 0.0, 1.0 );
	float r_A = ( CoC * 2.0 - 1.0 ) * uMaxCoCRadiusPixels;
	float nearFieldness_A = saturate( r_A * 4.0 );
#else 
	float CoC  = blurSource.a;	
	float r_A = ( CoC ) * uMaxCoCRadiusPixels;
	float nearFieldness_A = ( r_A );
#endif
	for( int delta = -uMaxCoCRadiusPixels; delta <= uMaxCoCRadiusPixels; ++delta )
    {
        // Tap location near A
        ivec2   B = A + ( kDirection * delta );

        // Packed values
        vec4 blurInput = texelFetch( uBlurSource, clamp( B, ivec2( 0 ), textureSize( uBlurSource, 0 ) - ivec2( 1 ) ), 0 );

        // Signed kernel radius at this tap, in pixels
        float r_B = ( CoC * 2.0 - 1.0 ) * float( uMaxCoCRadiusPixels );

        /////////////////////////////////////////////////////////////////////////////////////////////
        // Compute blurry buffer

        float weight = 0.0;
        
        float wNormal  = 
            // Only consider mid- or background pixels (allows inpainting of the near-field)
            float(! inNearField(r_B)) * 
            
            // Only blur B over A if B is closer to the viewer (allow 0.5 pixels of slop, and smooth the transition)
			// This term avoids "glowy" background objects but leads to aliasing under 4x downsampling
            saturate( abs( r_A ) - abs( r_B ) + 1.5 ) *
            
            // Stretch the kernel extent to the radius at pixel B.
            kernel[clamp( int( float( abs( delta ) * ( KERNEL_TAPS - 1 ) ) / ( 0.001 + abs( r_B * 0.8 ) ) ), 0, KERNEL_TAPS )];

        weight = mix( wNormal, 1.0, nearFieldness_A );

        // far + mid-field output 
        blurWeightSum  += weight;
        blurResult.rgb += blurInput.rgb * weight;
        
        ///////////////////////////////////////////////////////////////////////////////////////////////
        // Compute near-field super blurry buffer
        
        vec4 nearInput;

#if HORIZONTAL
            // On the first pass, extract coverage from the near field radius
            // Note that the near field gets a box-blur instead of a kernel 
            // blur; we found that the quality improvement was not worth the 
            // performance impact of performing the kernel computation here as well.

            // Curve the contribution based on the radius.  We tuned this particular
            // curve to blow out the near field while still providing a reasonable
            // transition into the focus field.
            nearInput.a = float( abs( delta ) <= r_B ) * saturate( r_B * uInvNearBlurRadiusPixels * 4.0 );
			
            // Optionally increase edge fade contrast in the near field
			nearInput.a *= nearInput.a; nearInput.a *= nearInput.a;

            // Compute premultiplied-alpha color
            nearInput.rgb = blurInput.rgb * nearInput.a;            
#else
            // On the second pass, use the already-available alpha values
            nearInput = texelFetch( uNearSource, clamp( B, ivec2( 0 ), textureSize( uNearSource, 0 ) - ivec2( 1 ) ), 0 );
#endif
                    
        // We subsitute the following efficient expression for the more complex: weight = kernel[clamp(int(float(abs(delta) * (KERNEL_TAPS - 1)) * uInvNearBlurRadiusPixels), 0, KERNEL_TAPS)];
        weight =  float( abs( delta ) < uNearBlurRadiusPixels );
        nearResult += nearInput * weight;
        nearWeightSum += weight;
    }
    
    blurResult.a = CoC;

    // Normalize the blur
    blurResult.rgb /= blurWeightSum;
	nearResult     /= max( nearWeightSum, 0.00001 );
}
