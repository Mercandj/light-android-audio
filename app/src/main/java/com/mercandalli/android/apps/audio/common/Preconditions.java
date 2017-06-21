package com.mercandalli.android.apps.audio.common;

import java.util.Collection;

/**
 * Static methods to be called at the Start of your methods to verify correct arguments/states.
 * <p/>
 * Do not necessary test all your arguments,
 * but a good practice is to test the arguments of the public API of your classes =)
 */
public final class Preconditions {

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
    public static void checkNotNull(Object object) {
        checkNotNullInternal(object, DEFAULT_DETAIL_MESSAGE_FOR_CHECK_NULL);
    }

    public static void checkNotNullAllCollection(Collection object) {
        checkNotNullInternal(object, DEFAULT_DETAIL_MESSAGE_FOR_CHECK_NULL);
        for (final Object o : object) {
            checkNotNullInternal(o, DEFAULT_DETAIL_MESSAGE_FOR_CHECK_NULL);
        }
    }

    /**
     * Ensures that an object reference is not null, with an error message.
     *
     * @param object  the {@link Object} to check.
     * @param message the given detail message.
     */
    public static void checkNotNull(Object object, String message) {
        checkNotNullInternal(object, message);
    }

    private static void checkNotNullInternal(Object object, String message) {
        if (object == null) {
            throw new NullPointerException(message);
        }
    }
}
