void native printf(string format, ... rest);
void* native calloc(int count, int elem_size);
void native free(void* buffer);

int main() {
	int c = ^(int x, int y) => { return x + y; }(5, 6);
	printf("%d", c);
	return 0;
}
