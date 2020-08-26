// login.js
.pragma library

/*
  Validate the input of User
  Return values are..
  True - Success
  False - credentials are empty
*/
function validateUser(uname) {
    return uname !== ""
}

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
