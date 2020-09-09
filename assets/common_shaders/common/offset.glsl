uniform vec2 uOffset;
uniform vec2 uDims;

vec2 calcTexCoordFromFrag( vec2 fragCoord )
{
	return ( fragCoord + uOffset ) / uDims;
}

vec2 calcTexCoordFromUv( vec2 uv )
{
	vec2 offset	= uOffset / uDims;
	uv			*= vec2( 1.0 ) - offset * 2.0;
	uv			+= offset;
	return uv;
}

