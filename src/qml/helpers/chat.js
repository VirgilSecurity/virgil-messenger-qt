.pragma library

function getPersonInitialis(nickname) {
    const arr = nickname.replace("_", "").split(/[\s_]/);
    const initials = arr.length > 1 ? arr[0][0] + arr[1][0] : arr[0][0] + arr[0][1];
    return initials ? initials.toUpperCase() : ""; // FIXME(fpohtmeh): remove
}
