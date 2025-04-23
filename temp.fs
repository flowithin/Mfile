/ (type d) (inode block 0)
	owner: 
	size: 1
	data disk blocks: 1 
	entry 0: dir, inode block 2

/dir (type d) (inode block 2)
	owner: user1
	size: 1
	data disk blocks: 3 
	entry 0: file.txt, inode block 4
	entry 1: dir_dir, inode block 6
	entry 2: dd1, inode block 11

/dir/file.txt (type f) (inode block 4)
	owner: user1
	size: 1
	data disk blocks: 5 
We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying

/dir/dir_dir (type d) (inode block 6)
	owner: user1
	size: 1
	data disk blocks: 7 
	entry 0: ddd, inode block 8

/dir/dir_dir/ddd (type d) (inode block 8)
	owner: user1
	size: 0
	data disk blocks: 

/dir/dd1 (type d) (inode block 11)
	owner: user1
	size: 1
	data disk blocks: 12 
	entry 0: dd2, inode block 13

/dir/dd1/dd2 (type d) (inode block 13)
	owner: user1
	size: 1
	data disk blocks: 14 
	entry 0: dd3, inode block 15

/dir/dd1/dd2/dd3 (type f) (inode block 15)
	owner: user1
	size: 1
	data disk blocks: 18 
hhhWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, lay

4081 disk blocks free
