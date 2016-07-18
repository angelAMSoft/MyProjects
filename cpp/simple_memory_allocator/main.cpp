#include <iostream>
#include <list>
#include <map>

#define FREE_BLOCK 1
#define USED_BLOCK 0
#define CHUNK_SIZE 32
#define POOL_SIZE 1048576

struct Node
{
	int offset;
	int status;
	int size;
	Node *next;
	Node *prev;
	Node *End;
};

typedef Node *PNode;

PNode CreateNode(int _offset, int _status, int _size)
{
PNode NewNode= (PNode) malloc(sizeof(Node));
NewNode->next = NULL;
NewNode->End = NULL;
NewNode->offset = _offset;
NewNode->size = _size;
NewNode->status = _status;
return NewNode;
}

class SmallAllocator {
private:
        char Memory[POOL_SIZE];
		PNode Head;
		std::map<char*, int> usedBlocks;
		void InitMemory()
		{

			int TotalSize = POOL_SIZE;
			int chunks = TotalSize / CHUNK_SIZE;
			PNode prevNode = NULL;
			for(int i = 0; i < chunks; ++i)
			{
				Node *chunk = CreateNode(i * CHUNK_SIZE, FREE_BLOCK, CHUNK_SIZE);
				if(Head == NULL)
				{
					Head = chunk;
					Head->prev = NULL;
				}
				else
				{
					prevNode->next = chunk;
					chunk->prev = prevNode;
				}
				prevNode = chunk;
			}
		}
		void* AllocMoreChunks(unsigned int Size)
		{
			PNode q, beginChunk, endChunk; //q end chunk
			q = Head; 
			int totalSize = 0;
			bool founded = false;
			while(q != NULL)
			{
				if(q->status == FREE_BLOCK)
				{	
					beginChunk = q;
					while(q != NULL && totalSize < Size)
					{
						if(q->next == NULL)
							break;
						if(q->status == USED_BLOCK)
						{
							beginChunk = NULL;
							endChunk = NULL;
							totalSize = 0;
							break;
						}
						totalSize += q->size;
						q = q->next;
					}
					endChunk = q;
					break;
				}
				else q = q->next;
			}
			if(q == NULL || totalSize < Size)
				return NULL;
			q = beginChunk;
			while(beginChunk != endChunk)
			{
				beginChunk->status = USED_BLOCK;
				beginChunk = beginChunk->next;
			}
			q->End = endChunk;
			char* addr = &Memory[q->offset];
			usedBlocks.insert(std::pair<char*,int>(addr,Size));
			return (void*)addr;
		}

public:
		SmallAllocator()
		{
			Head = NULL;
			InitMemory();
		}
		~SmallAllocator()
		{
			PNode q;
			PNode p;
			q = Head;
			while(q != NULL)
			{
				p = q->next;
				free(q);
				q = p;
			}
			Head = NULL;
		}
        void *Alloc(unsigned int Size) {
			if(Size > CHUNK_SIZE)
				return AllocMoreChunks(Size);
			PNode q;
			q = Head;
			while(q != NULL)
			{
				if(q->status == FREE_BLOCK)
					break;
				q = q->next;
			}
			if (q == NULL)
				return NULL;
			q->status = USED_BLOCK;
			q->End = q->next;
			char* addr = &Memory[q->offset];
			usedBlocks.insert(std::pair<char*,int>(addr,Size));
			return (void*)addr;
		}
        void *ReAlloc(void *Pointer, unsigned int Size) 
		{
			int oldSize = usedBlocks[(char*)Pointer];
			if(oldSize > Size)
				return Pointer;
			void* NewMem = Alloc(Size);
			if(NewMem == NULL)
				return NULL;
			memcpy(NewMem, Pointer, oldSize);
			Free(Pointer);			
			return NewMem;
		}
        void Free(void *Pointer) 
		{
			if(Pointer == NULL)
				return;
			PNode q, end;
			q = Head;
			char* mem = (char*)&Memory;
			int offset = (char*)Pointer - mem;
			while(q != NULL)
			{
				if(q->offset == offset)
					break;
				q = q->next;
			}
			if(q == NULL)
				return;
			end = q->End;
			while(q != end)
			{
				q->status = FREE_BLOCK;
				q = q->next;
			}
			std::map<char*,int>::iterator it;
			it = usedBlocks.find((char*)Pointer);
			usedBlocks.erase(it);
		}
};

int main()
{
	SmallAllocator A1;
	int * A1_P1 = (int *) A1.Alloc(sizeof(int));
	A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
	A1.Free(A1_P1);
	SmallAllocator A2;
	int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
	int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
	for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
	for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
	A2.Free(A2_P1);
	A2.Free(A2_P2);
	return 0;
}