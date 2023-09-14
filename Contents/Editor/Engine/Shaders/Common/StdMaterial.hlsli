
#define	MATERIAL_FLAG_ALBEDO_MAP (1 << 0)
#define MATERIAL_FLAG_NORMAL_MAP (1 << 1)
#define	MATERIAL_FLAG_SPECULAR_MAP (1 << 2)
#define	MATERIAL_FLAG_EMISSIVE_MAP (1 << 3)

struct PerMaterialData
{
	float3 Albedo;
	uint AlbedoMapIndex;
	
	float3 Specular;
	uint SpecularMapIndex;
	
	float3 Emissive;
	uint EmissiveMapIndex;
	
	uint NormalMapIndex;
	
	// MATERIAL_FLAG_*
	uint Flags;
};