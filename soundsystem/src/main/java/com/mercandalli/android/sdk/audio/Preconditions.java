package com.mercandalli.android.sdk.audio;

/**
 * Static methods to be called at the Start of your methods to verify correct arguments/states.
 * <p/>
 * Do not necessary test all your arguments,
 * but a good practice is to test the arguments of the public API of your classes =)
 */
/* package */
final class Preconditions {

    private Preconditions() {
        // Non-instantiable.
    }

    /**
     * A default detail message thrown with the {@link NullPointerException} in
     * {@link Preconditions#checkNotNullInternal(Object, String)}
     */
    private static final String DEFAULT_DETAIL_MESSAGE_FOR_CHECK_NULL = "Object can not be null.";

    /**
     * Ensures that an object reference is not null.
     *
     * @param object the {@link Object} to check.
     */
    /* package */
    static void checkNotNull(Object object) {
        checkNotNullInternal(object, DEFAULT_DETAIL_MESSAGE_FOR_CHECK_NULL);
    }

    private static void checkNotNullInternal(Object object, String message) {
        if (object == null) {
            throw new NullPointerException(message);
        }
    }
}
