namespace speck{
	struct speckage{
	}

	bool addFileToPackage(speckage& speckage, char* filepath);
	bool savePackageToFile(const speckage& toSave, char* filepath);
	speckage readPackageFromFile(char* filepath);
	void* readFileFromPackage(const speckage& speckage, char* filepath, uint32_t& size);
}
