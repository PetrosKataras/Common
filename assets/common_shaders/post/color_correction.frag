
uniform float	uBlend;
uniform float	uBrightness;
uniform float	uChromaticAberration;
uniform float	uContrast;
uniform float	uExposure;
uniform vec3	uSaturation;

uniform sampler2D uSampler;

layout (location = 0) out vec4 oColor;

in VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexIn;

void main( void )
{
	vec3 color	= vec3( 0.0 );
	//if ( uChromaticAberration != 0.0 ) {
		color.r = texture( uSampler, vec2( vVertexIn.texCoord.x + uChromaticAberration,	vVertexIn.texCoord.y ) ).r;
		color.g = texture( uSampler, vec2( vVertexIn.texCoord.x + 0.0,					vVertexIn.texCoord.y ) ).g;
		color.b	= texture( uSampler, vec2( vVertexIn.texCoord.x - uChromaticAberration,	vVertexIn.texCoord.y ) ).b;
	//}
	color		*= uSaturation;
	color.rgb	+= vec3( uBrightness );
	color		= clamp( color * 0.5 + 0.5 * color * color * uContrast, 0.0, 1.0 );
	color		= pow( color, vec3( uExposure ) );
	if ( uBlend < 1.0 ) {
		color	= mix( texture( uSampler, vVertexIn.texCoord.xy ).rgb, color, uBlend );
	}
	oColor		= vec4( color, 1.0 );
}
