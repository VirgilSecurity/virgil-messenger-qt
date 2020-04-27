
function chunk(arr, size) {
	return arr.reduce((storage, curr, index) => {
			const currentChunk = Math.floor(index / size)
			const indexInGroup = index % size
			if (!storage[currentChunk]) storage[currentChunk] = []
			storage[currentChunk][indexInGroup] = curr
			return storage
	}, [])
}


const res = chunk(['1'], 4);
console.log('res', res)