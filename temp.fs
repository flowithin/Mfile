/ (type d) (inode block 0)
	owner: 
	size: 1
	data disk blocks: 2 
	entry 0: dir, inode block 1

/dir (type d) (inode block 1)
	owner: user1
	size: 1
	data disk blocks: 4 
	entry 0: dir2, inode block 3

/dir/dir2 (type d) (inode block 3)
	owner: user1
	size: 1
	data disk blocks: 6 
	entry 0: dir3, inode block 5

/dir/dir2/dir3 (type d) (inode block 5)
	owner: user1
	size: 0
	data disk blocks: 

4089 disk blocks free
