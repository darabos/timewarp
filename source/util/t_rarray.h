template <class Type, int extra = 0> 
class R_Array {
	public:
	Type *array;
	int num, max;
	void preinit() {
		array = NULL;
		num = max = 0;
	}
	void deinit() {
		if (array) free(array);
		preinit();
	}
	~R_Array() {
		deinit();
	}
	void resize( int new_size ) {
		if (new_size > max) {
			max = new_size + extra;
			array = (Type*)realloc(array, max * sizeof(Type));
		}
	}
	Type operator[](int index) {
		return array[index];
	}
};
template <class Type, int extra = 0> 
class R_Ref_Array : public R_Array<Type,extra> {
	public:
	void add(Type ref) {
		int i = num;
		if (num == max) resize(num+1);
		array[i] = ref;
	}
	void clean() {
		int i = 0, j = 0;
		while (i < num - j) {
			if (!array[i]) {
				j += 1;
				continue;
			}
			else array[i] = array[i+j];
			i += 1;
		}
		num -= j;
	}
	~R_Ref_Array() {deinit();}
};

