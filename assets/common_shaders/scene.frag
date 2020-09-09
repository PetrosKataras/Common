#version 150

in VertexData {
	vec4 position;
	vec3 normal;
	vec4 color;
	vec2 texCoord;
	vec4 proj;
} vVertexIn;

out vec4 fragColor;

void main()
{
	vec2 uv = vVertexIn.texCoord;
	vec3 L = normalize( -vVertexIn.position.xyz );
	vec3 E = normalize( -vVertexIn.position.xyz );
	vec3 R = normalize( -reflect( L, vVertexIn.normal ) );

	// diffuse term with fake ambient occlusion
	float occlusion = 0.5 + 0.5*16.0*uv.x*uv.y*( 1.0 - uv.x )*( 1.0 - uv.y );
	vec4 diffuse = vVertexIn.color * occlusion * max( dot( vVertexIn.normal, L ), 1.0 );

	// specular term
	vec4 specular = vVertexIn.color * pow( max( dot( R, E ), 0.0 ), 50.0 );

	// write gamma corrected final color
	fragColor.rgb = sqrt( diffuse + specular ).rgb;
	fragColor.a = 1.0;
#if 0
	// set diffuse and specular colors
	vec3 cDiffuse = vVertexIn.color.rgb;
	vec3 cSpecular = cDiffuse * 0.5;// vec3( 1.0 );

	// light properties in view space
	vec3 vLightPosition = vec3( 0.0, 2.0, 5.0 );

	// lighting calculations
	vec3 N = normalize( vVertexIn.normal );
	vec3 L = normalize( vLightPosition - vVertexIn.position.xyz );
	vec3 E = normalize( -vVertexIn.position.xyz );
	vec3 H = normalize( L + E );

	// Calculate coefficients.
	float phong = max( dot( N, L ), 0.0 );

	const float kMaterialShininess = 1.0;
	const float kNormalization = ( kMaterialShininess + 8.0 ) / ( 3.14159265 * 8.0 );
	float blinn = pow( max( dot( N, H ), 0.0 ), kMaterialShininess ) * kNormalization;

	// diffuse coefficient
	vec3 diffuse = vec3( phong ) * cDiffuse;

	// specular coefficient 
	vec3 specular = blinn * cSpecular;

	// alpha 
	float alpha = 1.0;

	// final color
	fragColor = vec4( diffuse + specular, alpha );
	//fragColor = vVertexIn.color;
#endif
}
