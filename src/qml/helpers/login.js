// login.js
.pragma library

/*
  Creates an array of elements split into groups the length of size.
  If array can't be split evenly, the final chunk will be the remaining elements.
*/

function chunk(arr, size) {
    return arr.reduce((storage, curr, index) => {
        const currentChunk = Math.floor(index / size)
        const indexInGroup = index % size
        if (!storage[currentChunk]) storage[currentChunk] = []
        storage[currentChunk][indexInGroup] = curr
        return storage
    }, [])
}
// TODO(fpohtmeh): remove library
